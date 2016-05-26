<?php

/**
 * @file
 * Functions required in processing a spreadsheet.
 */

/**
 * Load the Spreadsheet into the database. This should be executed as a Tripal Job.
 *
 * @param $xls_file
 *   The spreadsheet containing phenotypics data to be loaded.
 * @param $plantprop_headers
 *   An array of columns that should be saved as plant properties rather than measurements.
 * @param $job_id
 *   The tripal job id.
 */
function rawpheno_load_spreadsheet($active_project, $arr_newheaders, $fid, $plantprop_headers, $job_id) {
  // Unserialize and remove formating for the headers that should be saved as 
  // plant properties rather than measurements.
  $plantprop_headers = unserialize($plantprop_headers);
  $arr_newheaders = unserialize($arr_newheaders);
  
  // First we load the file.
  $file = file_load($fid);
  
  if (!$file) {
    tripal_report_error(
      'rawpheno',
      TRIPAL_CRITICAL,
      'Uploading Phenoypic Data: Unable to load file. File ID=!id',
      array('!id' => $fid),
      array('print' => TRUE)
    );
    tripal_report_error('rawpheno', TRIPAL_CRITICAL, 'Failed to load phenoypic data (job !id)', 
                         array('!id' => $job_id), array('print' => TRUE));    
    exit(1);
  }
  
  $xls_file = drupal_realpath($file->uri);

  // Keep a record of which file we are loading in the logs.
  print "\nXLSX File: " . $xls_file . "\n";
  
  // Add the libraries needed to parse excel files.
  rawpheno_add_parsing_libraries();

  // Open the file for reading
  $xls_obj = rawpheno_open_file($file);
  if (!$xls_obj) {
    tripal_report_error(
      'rawpheno',
      TRIPAL_CRITICAL,
      'Uploading Phenoypic Data: Unable to open file. File=@file',
      array('@file' => print_r($file, TRUE)),
      array('print' => TRUE)
    );
    tripal_report_error('rawpheno', TRIPAL_CRITICAL, 'Failed to load phenoypic data (job !id)', 
                         array('!id' => $job_id), array('print' => TRUE));
    exit(2);
  }

  // Change to the correct spreadsheet.
  rawpheno_change_sheet($xls_obj, 'measurements');

  // File to write progress status.         
  $tmp = file_directory_temp();
  $filename = $tmp . '/' . 'job-progress' . $job_id . '.txt';
  // Count all the rows in the spreadsheet and use this count to compute
  // the percentage of the currently saved rows.
  $row_count = rawpheno_count_rows($xls_obj);
    
  // Start Transaction.
  $TRANSACTION = db_transaction();
  try {
  
    // Read each row.
    print "\nNow parsing each row and saving it to the database...\nNumber of rows saved: \n";
    $i = 0;

    // Each row in the spreadsheet.
    foreach ($xls_obj as $row) {
      // print update every 25 lines.
      if (($i % 25) == 0 AND $i !== 0) {
        print $i . " of " . $row_count . ' - ' . $percent . '%'."\n";
        
        $percent = round(($i/$row_count) * 100);
        file_unmanaged_save_data($percent, $filename, FILE_EXISTS_REPLACE);
      }
      
      // Update job status every 5%
      // There is no point doing this because the transaction ensures 
      // we don't see any results until the job is done...
      // $percent_complete = round(($i / $num_rows) * 100);
      //if (($percent_complete % 5) == 0) {
      //  tripal_set_job_progress($job_id, $percent_complete);
      //}
      
      // HEADER!
      //print_r($row);
      // This is the header.
      if ($i == 0) {
        $header = $row;

        // Find the index number of name header in the spreadsheet.
        $name_index = array_search('name', array_map('rawpheno_function_delformat', $header));
        if (!$name_index) {
          tripal_report_error(
            'rawpheno',
            TRIPAL_CRITICAL,
            'Uploading Phenoypic Data: Uname to determine the name column.',
            array(),
            array('print' => TRUE)
          );
          $TRANSACTION->rollback();
          tripal_report_error('rawpheno', TRIPAL_CRITICAL, 'Failed to load phenoypic data (job !id)', array('!id' => $job_id), array('print' => TRUE));
          exit(31);
        }

        $i++;
        continue;
      }

      // NEXT ROW IF EMPTY!
      // Don't continue processing if this row is empty.
      if (strlen(trim(implode('', $row))) <= 1) {
        break;
      }
      
      // VALID EACH ROW!
      // Process the name column first since we need a plant_id before we can insert any more data.
      // Name column header goes into pheno_plant.
      // Check if stock name exists.
      $stock_id = NULL;
      $r = chado_query('SELECT stock_id FROM {stock} WHERE name=:name', array(':name' => trim($row[$name_index])));
      if ($r) $stock_id = $r->fetchField();

      // Determine if name has a stock id number.
      if (isset($stock_id) && $stock_id > 0) {
        // Plant_id for this row.
        $pheno_plantid = db_insert('pheno_plant')
          ->fields(array('stock_id' => $stock_id))
          ->execute();
      
        // Map this record to a project.
        db_insert('pheno_project_stock')
          ->fields(array('project_id' => $active_project,
                         'stock_id' => $pheno_plantid))
          ->execute();               
      } 
      else {
        // Warn the admin tht germplasm is not available...  
        // We want to stop loading if this is the case.
        tripal_report_error(
          'rawpheno',
          TRIPAL_CRITICAL,
          'Uploading Phenoypic Data: Germplasm doesn\'t exist (name=!name; row=!row)',
          array('!name' => $row[$name_index], '!row' => $i),
          array('print' => TRUE)
        );
        $TRANSACTION->rollback();
        tripal_report_error('rawpheno', TRIPAL_CRITICAL, 'Failed to load phenoypic data (job !id)', array('!id' => $job_id), array('print' => TRUE));
        exit(32);
      }
      
      // Read each row and each cell.
      // Each row will be an array where name is always the first element.
      foreach($row as $cell_index => $cell_entry) {
        // We don't want to insert empty data.
        // That said, while PHP thinks 0 is empty, we do not.
        if (!empty($cell_entry) OR (strval($cell_entry) === '0')) {

          // Get the column header of a cell.
          $cell_colheader = str_replace(array("\n", "\r", "  "), ' ', $header[$cell_index]);
          
          // Determine if user wants to save this trait.
          if (count($arr_newheaders) > 0 AND array_key_exists($cell_colheader, $arr_newheaders)) {
            if ($arr_newheaders[$cell_colheader] == 0) {
              // Skip this cell if it is a new column header and user does not want to save
              // this new trait;
              continue;
            }
          }
          
          // We always want to strip flanking white space.
          // FYI: This is done when the data is validated as well.
          $cell_entry = trim($cell_entry);
          $cell_colheader = trim($cell_colheader);
          
          // Determine which table to insert a column header.
          // If this is the name column then doing nothing since we've already delt with it above.
          if ($cell_index == $name_index) { continue; } 
          
          // PLOT, ENTRY, REP and LOCATION
          // Cells containing column headers that are required.
          // Traits: plot, entry, rep, location into pheno_plantprop.
          elseif (in_array($cell_colheader, $plantprop_headers) && !empty($cell_colheader)) {
            
            $type = tripal_get_cvterm(array('name' => $cell_colheader, 'cv_id' => array('name' => 'phenotype_plant_property_types')));
            $type_id = $type->cvterm_id;
        
            // Ensure that cvterm_id is present before inserting to table
            if(isset($type_id)) {
              $tmp = db_insert('pheno_plantprop')
                ->fields(array('plant_id' => $pheno_plantid, 
                               'type_id' => $type_id, 
                               'value' => $cell_entry))
                ->execute();
          
              if (!$tmp) {
                tripal_report_error(
                  'rawpheno',
                  TRIPAL_ERROR,
                  'Uploading Phenoypic Data: Unable to insert plant property. Values=@values',
                  array('@values' => print_r(array('plant_id' => $pheno_plantid,  'type_id'  => $type_id,  'value' => $cell_entry),TRUE)),
                  array('print' => TRUE)
                );
                $TRANSACTION->rollback();
                tripal_report_error('rawpheno', TRIPAL_CRITICAL, 'Failed to load phenoypic data (job !id)', array('!id' => $job_id), array('print' => TRUE));
                exit(33);
              }
            }
          
            else {
              tripal_report_error(
                'rawpheno',
                TRIPAL_ERROR,
                'Uploading Phenoypic Data: Plant Property type !type does\'t exist.',
                array('!type' => $cell_colheader),
                array('print' => TRUE)
              );
              $TRANSACTION->rollback();
              tripal_report_error('rawpheno', TRIPAL_CRITICAL, 'Failed to load phenoypic data (job !id)', array('!id' => $job_id), array('print' => TRUE));
              exit(34);
            }
          
          } 
          // THE REST OF THE COLUMN HEADERS
          // Everything else into pheno_measurements.
          elseif (!empty($cell_colheader)) {
            // Get the cvterm_id for the trait measurement.
            $type_id = rawpheno_get_trait_id($cell_colheader);
            if (!$type_id) {
              tripal_report_error(
                'rawpheno',
                TRIPAL_ERROR,
                'Uploading Phenoypic Data: Missing Plant Measurement Type (Header=!colheader).',
                array('!colheader' => $cell_colheader),
                array('print' => TRUE)
              );
              $TRANSACTION->rollback();
              tripal_report_error('rawpheno', TRIPAL_CRITICAL, 'Failed to load phenoypic data (job !id)', array('!id' => $job_id), array('print' => TRUE));
              exit(37);
            }
          
            // Retrieve the unit for this trait.
            $cv_unit = rawpheno_get_trait_unit($cell_colheader, $type_id);
            if ($cv_unit) {
              $unit_id = $cv_unit['id'];
              $unit = $cv_unit['name'];
            }
            else {
              tripal_report_error(
                'rawpheno',
                TRIPAL_ERROR,
                'Uploading Phenoypic Data: Unable to find unit for Plant Measurement Type (Term=!name; Type ID=!id).',
                array('!name' => $cv_name, '!id' => $type_id),
                array('print' => TRUE)
              );
              $TRANSACTION->rollback();
              tripal_report_error('rawpheno', TRIPAL_CRITICAL, 'Failed to load phenoypic data (job !id)', array('!id' => $job_id), array('print' => TRUE));
              exit(37);
            }
      
            // Determine if cell requires scale member code.
            // When unit is scale, find code equivalent in pheno_scale_member table.  
            if ($unit == 'scale') {
              // Get pheno scale member code 
              $cvalue_id = db_query("SELECT member_id FROM {pheno_scale_member} 
                                   WHERE code = :code LIMIT 1", 
                                   array(':code' => trim($cell_entry)))
              ->fetchField(); 
              // We want to report an error if we can't find the scale memeber 
              // but only if there are any in the first place!
              $num_members = db_query('SELECT count(*) FROM {pheno_scale_member} WHERE scale_id=:unit_id',
                array(':unit_id' => $unit_id))->fetchField();
              if (!$cvalue_id AND !empty($num_members)) {
                tripal_report_error(
                  'rawpheno',
                  TRIPAL_WARNING,
                  'Uploading Phenoypic Data: Unable to find scale id for Plant Measurement Type (Trait=!trait; Term=!name; Type ID=!id; Scale Value=!scale).',
                  array('!trait' => $cell_colheader, '!name' => $unit, '!id' => $unit_id, '!scale' => $cell_entry),
                  array('print' => TRUE)
                );
              }
          
              // Use default value in the cell if query to find scale member code 
              // has no equivalent value.
              $cvalue_id = (isset($cvalue_id) && $cvalue_id > 0) ? $cvalue_id : $cell_entry;
            } 
            else {
              // No scale member value for the rest of traits.
              $cvalue_id = '';
            }

            // Insert trait only when type_id and unit_id are not null.
            if (isset($type_id) && isset($unit_id)) {
            
              $temp = db_insert('pheno_measurements')
                ->fields(array('plant_id' => $pheno_plantid, 
                               'type_id' => $type_id, 
                               'unit_id' => $unit_id, 
                               'cvalue_id' => $cvalue_id,
                               'value' => $cell_entry,
                               'modified' => date("D M d, Y h:i:s a", time())))
                ->execute();
                
                if (!$temp) {
                tripal_report_error(
                  'rawpheno',
                  TRIPAL_ERROR,
                  'Uploading Phenoypic Data: Unable to insert measurement. Values=@values.',
                  array('@values' => print_r(array('plant_id'  => $pheno_plantid, 
                             'type_id' => $type_id, 
                             'unit_id' => $unit_id, 
                             'cvalue_id' => $cvalue_id,
                             'value' => $cell_entry,
                             'modified' => date("D M d, Y h:i:s a", time())),TRUE)),
                  array('print' => TRUE)
                );
                $TRANSACTION->rollback();
                tripal_report_error('rawpheno', TRIPAL_CRITICAL, 'Failed to load phenoypic data (job !id)', array('!id' => $job_id), array('print' => TRUE));
                exit(38);
              }
            }
          }  
        }
      }
      
      $i++;
    }
  }
  catch (Exception $e) {
    $TRANSACTION->rollback();
    watchdog_exception('rawpheno', $e);
    tripal_report_error('rawpheno', TRIPAL_CRITICAL, 'Failed to load phenoypic data (job !id)', array('!id' => $job_id), array('print' => TRUE));
    exit(4);
  }  
  
  unset($TRANSACTION); //Commit
  print "Complete.\n";
  
  print "\nUpdating the materialized view summarizing phenotypic data.\n";
  $mview_id = tripal_get_mview_id('rawpheno_rawdata_summary');
  if ($mview_id) tripal_populate_mview($mview_id);
}


/**
 * Validates an excel file using any validators registered with rawpheno.
 *
 * @param $file
 *   A drupal managed_file object describing the uploaded spreadsheet.
 * @param $project_id
 *   An integer containing project id selected in the project select box.
 *   This will map the data submitted to a project.
 */
function rawpheno_validate_excel_file($file, $project_id) {
  $status = array();
  
  // Process the validators to make them easier to use.
  // Specifically, sort them by their scope.
  $validators = array();
  $all_validators = module_invoke_all('rawpheno_validators');
  foreach($all_validators as $k => $v) {
    $validators[ $v['scope'] ][ $k ] = $v;
  }

  // Add the libraries needed to parse excel files.
  rawpheno_add_parsing_libraries();

  // Before performing any validation to the excel file. Ensure first that a
  // project is active or selected and ensure that a project meets the column headers
  // required (eg. plant property and at least with a column header either essential or options).
  foreach ($validators['project'] as $prj_validator_name => $prj_validator) {
    if (isset($prj_validator['validation callback']) AND function_exists($prj_validator['validation callback'])) {
      $status[ $prj_validator_name ] = call_user_func($prj_validator['validation callback'], $project_id);
      
      // If returned false then halt validation.
      if ($status[ $prj_validator_name ] === FALSE) {
        return $status;
      } 
    }
  }

  // First validate the whole file. If any of these fail then halt validation.
  foreach ($validators['file'] as $validator_name => $validator) {
    if (isset($validator['validation callback']) AND function_exists($validator['validation callback'])) {
      $status[ $validator_name ] = call_user_func($validator['validation callback'], $file);

      // If returned false then halt validation.
      if ($status[ $validator_name ] === FALSE) {
        return $status;
      }
    }
  }
  
  // Open the file for reading
  $xls_obj = rawpheno_open_file($file);

  // Change to the correct spreadsheet.
  rawpheno_change_sheet($xls_obj, 'measurements');
  
  // This increment variable $i is required since xls and xlsx
  // parsers assign array index differently. 
  // XLS starts at 1, while XLSX at 0;
  $i = 0;
  
  // Iterate though each row.
  $num_errored_rows = 0;
  $storage = array();
  foreach($xls_obj as $row) {
    $i++;
  
    // Convert row into a string and check the length.
    // This will exclude empty rows.        
    if (strlen(trim(implode('', $row))) >= 1) {
    
      // VALIDATE THE HEADER.
      if ($i == 1) {
        // Save the header for later.
        $header = array();
        $new_header = array();
        foreach ($row as $r) {
          $without_format = rawpheno_function_delformat($r);
          $no_units = preg_replace('/(\s+\(.*\))/','',$r);
          $header[] = array(
            'no format' => $without_format,
            'original' => $r,
            'units' => rawpheno_function_unit($without_format),
            'no units' => $no_units,
          );
        }
        
        // Foreach validator with a scope of header, execute the validation callback & save the results.
        foreach($validators['header'] as $validator_name => $validator) {
          if (isset($validator['validation callback']) AND function_exists($validator['validation callback'])) {
            $result = call_user_func($validator['validation callback'], $header);
            
            // The status needs to keep track of which rows failed for a given header.
            if ($result === FALSE) {
              $status[ $validator_name ] = $i;
            }
            elseif (is_array($result)) {
              $status[ $validator_name ] = $result;
            }
          }
        }
      } 
      // VALIDATE THE ROW.
      else {

        $row_has_error = FALSE;
        foreach ($row as $column_index => $cell) {
          
          $column_name = $header[$column_index]['no units'];
          if (empty($column_name)) continue;
          
          // We always want to strip flanking white space.
          // FYI: This is done when the data is loaded as well.
          $cell = trim($cell);
          
          // Foreach validator:
          foreach (array('all','subset') as $scope) {
            foreach($validators[$scope] as $validator_name => $validator) {
            
              // Only validate if there is a validation callback.
              if (isset($validator['validation callback']) AND function_exists($validator['validation callback'])) {
              
                // Only validate if the current validator applies to the current column.
                // Specifically, if there are no defined headers it's applicable to
                //   OR if the current header is in the list of applicable headers.
                if (!isset($validator['headers']) OR in_array($column_name, $validator['headers'])) {     
                    
                  // Execute the validation callback & save the results.
                  $tmp_storage = (isset($storage[$validator_name])) ? $storage[$validator_name] : array();
                  $context = array(
                    'row index' => $i,
                    'column index' => $column_index,
                    'row' => $row,
                    'header' => $header
                  );
                  $result = $validator['validation callback']($cell, $context, $tmp_storage);

                  // Note: we use tmp storage b/c passing $storage[$validator_name] directly
                  // doesn't seem to work.
                  $storage[$validator_name] = $tmp_storage;
                
                  // The status needs to keep track of which rows failed for a given header.
                  if (is_array($result)) {
                    $status[ $validator_name ][ $column_name ][$i] = $result;
                    $row_has_error = TRUE;
                  }
                  elseif ($result !== TRUE) {
                    $status[ $validator_name ][ $column_name ][$i] = $i;
                    $row_has_error = TRUE;
                  }
                }
              }
            }
          }
        }
        
        if ($row_has_error) $num_errored_rows++;
      }
      
      // Only check until you have 10 rows with errors.
      if ($num_errored_rows >= 10) {
      
        // We only want to present the warning if this is not the end of the file ;-)
        $has_next = $xls_obj->next();
        if ($has_next AND strlen(trim(implode('', $has_next))) >= 1) {
          drupal_set_message("We have only checked the first $i lines of your file. Please fix the errors reported below and then upload the fixed file.", 'error');
          return $status;
        }
      }
    }
  }
      
  // Make sure all validators are represented in status. 
  // If they are not already then a failure wasn't recorded -thus they passed :-).
  foreach($all_validators as $validator_name => $validator) {
    if (!isset($status[$validator_name])) $status[$validator_name] = TRUE;
  }

  return $status;
}

/**
 * Open the Excel file using the spreadsheet reader.
 *
 * @param $file
 *   A Drupal managed file object.
 * @return
 *   An object representing the Excel file.
 */
function rawpheno_open_file($file) {

  // Grab the path and extension from the file.
  $xls_file = drupal_realpath($file->uri);
  $xls_extension = pathinfo($file->filename, PATHINFO_EXTENSION);
  
  // Validate that the spreadsheet is either xlsx or xls and open the spreadsheet using
  // the correct class.
  // XLSX:
  if ($xls_extension == 'xlsx') {
    $xls_obj = new SpreadsheetReader_XLSX($xls_file);
  } 
  // XLS:
  elseif ($xls_extension == 'xls') {
    // PLS INCLUDE THIS FILE ONLY FOR XLS TYPE.
    $xls_lib = libraries_load('spreadsheet_reader');
    $lib_path = $xls_lib['path']; 

    include_once $lib_path . 'SpreadsheetReader_XLS.php';
    $xls_obj = new SpreadsheetReader_XLS($xls_file);
  }
  
  return $xls_obj;
}

/**
 * Changes the worksheet in the Excel Object.
 *
 * @param $xls_obj
 *   The object describing this Excel workbook.
 * @param $tab_name
 *   The name of the tab you would like to switch to.
 * @return
 *   TRUE if it found the tab and FALSE otherwise.
 */
function rawpheno_change_sheet(&$xls_obj, $tab_name) {

  $xls_sheets = $xls_obj->Sheets();
  foreach($xls_sheets as $sheet_key => $sheet_value) {
    $xls_obj->ChangeSheet($sheet_key);

    // Only process the measurements worksheet.
    if (rawpheno_function_delformat($sheet_value) == 'measurements') {
      return TRUE;
    }
  }
  return FALSE;
  
}

/**
 * Function to remove all formatting from a cell value.
 *
 * @param $xls_cell_value
 *   Contains a value of a cell.
 *
 * @return 
 *   Contains a cell value with all formatting removed.
 */
function rawpheno_function_delformat($xls_cell_value) {
  // Remove any extra spaces, new lines, leading and trainling spaces
  // and covert the final result to lowercase.
  return trim(strtolower(preg_replace('!\s+!', ' ', $xls_cell_value)));
}

/**
 * Function to extract the unit from the column header.
 *
 * @param $xls_header_cell
 *   A string containing a column header.
 *
 * @return
 *   A string containing the unit found from the column header.
 */
function rawpheno_function_unit($xls_header_cell) {
  // Remove all formatting.  
  $temp_value = rawpheno_function_delformat($xls_header_cell);
  
  // If this is a scale then return that.
  if (preg_match('/\(scale/',$temp_value)) {
    return 'scale';
  }
  
  // Remove the following characters.
  $cell_value = str_replace(array(';', '1st', '2nd', 'r1', 'r3', 'r5', 'r7', ': 1-5'), '', $temp_value);

  // Extract text information inside the parenthesis.
  preg_match("/.*\(([^)]*)\)/", $cell_value, $match);
  
  // Return unit found, or default to text if no unit.
  return (isset($match[1])) ? trim($match[1]) : 'text';
}

/**
 * Determines which headers indicate new traits as compared to those expected for AGILE.
 *
 * @param $xls_file
 *    The full path to the excel file containing data.
 * @param $handle?
 *    A reference to the openened file?
 *
 * @return
 *    An array of headers for new traits.
 */
function rawpheno_indicate_new_headers($file) {
  $active_project = $_SESSION['rawpheno']['active_project'];

  // Retrieve the header for the indicated file.
  rawpheno_add_parsing_libraries();
  $xls_obj = rawpheno_open_file($file);
  rawpheno_change_sheet($xls_obj, 'measurements');
  // Note: we use the foreach here 
  // because the library documentation doesn't have a single fetch function.
  foreach ($xls_obj as $xls_headers) { break; }
  
  // Array to hold epected column headers.
  //$expected_headers = rawpheno_function_headers('expected');
  $expected_headers = rawpheno_project_traits($active_project);
  
  // Remove any formatting in each column headers.
  $expected_headers = array_map('rawpheno_function_delformat', $expected_headers);
  
  // Array to hold new column headers.
  $new_headers = array();

  // Assuming the file actually has a non-empty header row...
  if (count($xls_headers) > 0) {
  
    // Read each column header and compare against expected column headers.
    foreach($xls_headers as $value) {
      $temp_value = rawpheno_function_delformat($value);
      
      // Determine if column header exists in the expected column headers.
      if (!in_array($temp_value, $expected_headers) && !empty($value)) {
        // Not in expected column headers, save it as new header.
        array_push($new_headers, $value);
      }
    }
  }

  return $new_headers;
}

/**
 * Get all column headers.
 *
 * @param $file
 *    The full path to the excel file containing data.
 *
 * @return
 *    An array of headers.
 */
function rawpheno_all_headers($file) {
  // Retrieve the header for the indicated file.
  rawpheno_add_parsing_libraries();
  $xls_obj = rawpheno_open_file($file);
  rawpheno_change_sheet($xls_obj, 'measurements');
  // Note: we use the foreach here 
  // because the library documentation doesn't have a single fetch function.
  
  $arr_headers = array();
  foreach ($xls_obj as $xls_headers) {
    foreach($xls_headers as $h) {
      if (strlen($h) > 2) {
        $arr_headers[] = trim($h);
      }
    }
    break;
  }
  
  return $arr_headers;
}

/**
 * Count all rows in a spreadsheet
 *
 * @param $data_sheet
 *    The tab/sheet containing all data.
 *
 * @return
 *    An integer value of the total rows.
 */
function rawpheno_count_rows($data_sheet) {
  $count_rows = 0;
  foreach ($data_sheet as $row) {
    if (strlen(implode('', $row)) > 2) {
      $count_rows++;
    }
  }
  
  // Less header row.
  return $count_rows - 1;
}

/**
 * Adds the necessary files for EXCEL parsing.
 */
function rawpheno_add_parsing_libraries($file_type = 'XLSX') {

  // Function call libraries_load() base on the implementation
  // of hook_libraries_info() in rawpheno.module.
  $xls_lib = libraries_load('spreadsheet_reader');
  // Library path information returned will be used
  // to include individual library files required.
  $lib_path = $xls_lib['path'];

  // Include parser library. PLS DO NOT ALTER ORDER!!!
  // To stop parser from auto formatting date to MM/DD/YY,
  // suggest a new date format YYYY-mm-dd in:
  //   line 678 in excel_reader2.php
  //   line 834 in SpreadsheetReader_XLSX.php
  include_once $lib_path . 'php-excel-reader/excel_reader2.php';
  include_once $lib_path . 'SpreadsheetReader_XLSX.php';
  include_once $lib_path . 'SpreadsheetReader.php';  

  if ($file_type == 'XLS') {
    // PLS INCLUDE THIS FILE ONLY FOR XLS TYPE.
    include_once $lib_path . 'SpreadsheetReader_XLS.php';
  }
}

/**
 * Retrieve the cvterm_id for a given header.
 *
 * @param $header
 *   The unchanged/original header text for the trait.
 * @return
 *   The cvterm_id for the trait.
 */
function rawpheno_get_trait_id($header) {
  
  $header = str_replace(array("\n", "\r", "  "), ' ', $header);
  
  $type = tripal_get_cvterm(array('name' => $header, 'cv_id' => array('name' => 'phenotype_measurement_types')));
  
  if ($type) {
    return $type->cvterm_id;
  }
  
  return FALSE;
}

/**
 * Retrieve the unit for the trait.
 *
 * @param $trait_name
 *   The name of the trait as found in the column header.
 * @param $trait_id
 *   The cvterm_id of the trait if you have it (OPTIONAL).
 * @return
 *   Returns an array with the cvterm_id and name of the unit.
 */
function rawpheno_get_trait_unit($trait_name, $trait_id = NULL) {

  // Get the trait id if that is not provided to us.
  if (!$trait_id) {
    $type_id = rawpheno_get_trait_id($trait_name);
  }
  
  // First we try to get the unit through relationships since that avoids making assumptions.
  // in chado.cvterm_relationship.
  $unit = db_query("SELECT rel.object_id as id, cvt.name as name 
    FROM chado.cvterm_relationship rel
    LEFT JOIN chado.cvterm cvt ON cvt.cvterm_id=rel.object_id
    WHERE rel.subject_id = :trait LIMIT 1",
    array(':trait' => $trait_id))->fetchAll();
  
  if (sizeof($unit) == 1) {
    return $unit[0];
  }

  // If that doesn't work then we try to extract it from the name.
  // Note: if the following function is unable to extract the unit then it will default to text.
  $unit_name = rawpheno_function_unit($trait_name);
  // Column header does not contain unit, use text as default
  $cvterm = tripal_get_cvterm(array('name' => $unit_name, 'cv_id' => array('name' => 'phenotype_measurement_units')));

  if ($cvterm) {
    return array('id' => $cvterm->cvterm_id, 'name' => $cvterm->name);
  }
  
  return FALSE;
}

/**
 * Remove the unit part from a trait.
 *
 * @param $trait_name
 *   A string containing the trait name as formatted in cvterm name.
 * @return 
 *   A string containing the trait name without the unit.
 */
function rawpheno_get_trait_name($trait_name) {
  $t = explode('(', $trait_name);

  // Given a trait as defined in cvterm name in the following format:
  // Trait name (Trait Rep; Unit), extract the trait name only and return 
  // the extracted name.
  return (count($t) > 1) ? trim(preg_replace('/\(.*/', ' ', $trait_name)) : $trait_name;
} 

/**
 * Get all the essential traits in a project selected.
 *
 * @param $project_id
 *   An integer containing the project ID number.
 * @return 
 *   An array containing all essential traits in a project.
 */ 
function rawpheno_project_essential_traits($project_id) {
  // Get array of trait types
  $trait_type = rawpheno_function_trait_types();
   
  // Array to hold trait names.
  $arr_essential_traits = array();
  
  // Query essential traits in a project.
  $sql = "SELECT t2.name AS cvterm
    FROM pheno_project_cvterms AS t1 
      INNER JOIN chado.cvterm AS t2 USING (cvterm_id) 
    WHERE t1.project_id = :project_id AND t1.type = :essential
    ORDER BY t2.name ASC";
   
  $args = array(':project_id' => $project_id, ':essential' => $trait_type['type1']);  
  $trait = chado_query($sql, $args);
  
  foreach($trait as $t) {
    $m = rawpheno_get_trait_name($t->cvterm);
    array_push($arr_essential_traits, $m);
  }
  
  // Add Name column header to the traits returned.
  array_push($arr_essential_traits, 'Name');
  
  return $arr_essential_traits;
}

/**
 * Get all the plant property traits in a project selected.
 *
 * @param $project_id
 *   An integer containing the project ID number.
 * @return 
 *   An array containing all essential traits in a project.
 */ 
function rawpheno_project_plantproperty_traits($project_id) {
  // Get array of trait types
  $trait_type = rawpheno_function_trait_types();
   
  // Array to hold trait names.
  $arr_plantproperty_traits = array();
  
  // Query essential traits in a project.
  $sql = "SELECT t2.name AS cvterm
    FROM pheno_project_cvterms AS t1 
      INNER JOIN chado.cvterm AS t2 USING (cvterm_id) 
    WHERE t1.project_id = :project_id AND t1.type = :plantproperty
    ORDER BY t2.name ASC";
   
  $args = array(':project_id' => $project_id, ':plantproperty' => $trait_type['type4']);  
  $trait = chado_query($sql, $args);
  
  foreach($trait as $t) {
    $m = rawpheno_get_trait_name($t->cvterm);
    array_push($arr_plantproperty_traits, $m);
  }
  
  return $arr_plantproperty_traits;
}

/**
 * Get all traits available in a project whether essential or not.
 *
 * @param $project_id
 *   An integer containing the project ID number.
 * @return 
 *   An array containing all traits available in a project.
 */
function rawpheno_project_traits($project_id) {
  $arr_trait = array();
  
  // Query essential traits in a project.
  $sql = "SELECT trim(t2.name) AS cvterm
    FROM pheno_project_cvterms AS t1 
      INNER JOIN chado.cvterm AS t2 USING (cvterm_id) 
    WHERE t1.project_id = :project_id
    ORDER BY t2.name ASC";
   
  $args = array(':project_id' => $project_id);  
  $trait = chado_query($sql, $args);
  
  foreach($trait as $t) {
    array_push($arr_trait, $t->cvterm);
  }
  
  // Add Name column header to the traits returned.
  array_push($arr_trait, 'Name');
  
  return $arr_trait;
}