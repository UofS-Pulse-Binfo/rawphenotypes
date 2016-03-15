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
function rawpheno_load_spreadsheet($fid, $plantprop_headers, $job_id) {

  // Unserialize and remove formating for the headers that should be saved as 
  // plant properties rather than measurements.
  $plantprop_headers = unserialize($plantprop_headers);
  $plantprop_headers = array_map('rawpheno_function_delformat', $plantprop_headers);
  
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
    tripal_report_error('rawpheno', TRIPAL_CRITICAL, 'Failed to load phenoypic data (job !id)', array('!id' => $job_id), array('print' => TRUE));
    exit(1);
  }
  $xls_file = drupal_realpath($file->uri);

  // Keep a record of which file we are loading in the logs.
  print "\nXLSX File: " . $xls_file . "\n";;
  
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
    tripal_report_error('rawpheno', TRIPAL_CRITICAL, 'Failed to load phenoypic data (job !id)', array('!id' => $job_id), array('print' => TRUE));
    exit(2);
  }

  // Change to the correct spreadsheet.
  rawpheno_change_sheet($xls_obj, 'measurements');

  //$num_rows = sizeof($xls_obj);
  //print "Number of rows: ".$num_rows.".\n";
  
  // Array of column headers with first and second try.
  $arr_multiheaders = rawpheno_function_headers('multi-trial');
  $arr_multiheaders = array_map('rawpheno_function_delformat', $arr_multiheaders);

  // Start Transaction.
  $TRANSACTION = db_transaction();
  try {
    // Read each row.
    print "\nNow parsing each row and saving it to the database...\nNumber of rows saved: ";
    $i = 0;
    foreach ($xls_obj as $row) {

      // print update every 25 lines.
      if (($i % 25) == 0 AND $i !== 0) {
        print $i . "... ";
      }
    
      // Update job status every 5%
      //$percent_complete = round(($i / $num_rows) * 100);
      //if (($percent_complete % 5) == 0) {
      //  tripal_set_job_progress($job_id, $percent_complete);
      //}
      
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
      
      // Don't continue processing if this row is empty.
      if (strlen(trim(implode('', $row))) <= 1) {
        break; break;
      }
      
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
        if (empty($cell_entry) AND !(strval($cell_entry) === 0)) {
          break; break;
        }
        
        // Get the column header of a cell.
        $cell_colheader = rawpheno_function_delformat($header[$cell_index]);
        
        // We always want to strip flanking white space.
        // FYI: This is done when the data is validated as well.
        $cell_entry = trim($cell_entry);

        // Determine which table to insert a column header.
        // If this is the name column then doing nothing since we've already delt with it above.
        if ($cell_index == $name_index) { } 
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
                             'type_id'  => $type_id, 
                             'value'    => $cell_entry))
              ->execute(); 
            if (!$tmp) {
              tripal_report_error(
                'rawpheno',
                TRIPAL_ERROR,
                'Uploading Phenoypic Data: Unable to insert plant property. Values=@values',
                array('@values' => print_r(array('plant_id' => $pheno_plantid,  'type_id'  => $type_id,  'value'    => $cell_entry),TRUE)),
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
        // Determine if column header is New or not
        elseif (!empty($cell_colheader)) {
          if (count($new_header) > 0 && in_array($cell_colheader, $new_header)) {
            // NEW
            // Test if column header of cell is a new trait.
            // Replace any extra spaces to underscore and create an exact match of
            // cvterm name inserted into cvterm table.
            $cv_name = str_replace(' ', '_', $cell_colheader);
            $type = tripal_get_cvterm(array('name' => $cv_name, 'cv_id' => array('name' => 'phenotype_measurement_types')));
            $type_id = $type->cvterm_id;
            if (!$type_id) {
              tripal_report_error(
                'rawpheno',
                TRIPAL_ERROR,
                'Uploading Phenoypic Data: Plant Measurement Type (New Trait; Term=!name; Header=!colheader) does\'t exist.',
                array('!name' => $cv_name, '!colheader' => $cell_colheader),
                array('print' => TRUE)
              );
              $TRANSACTION->rollback();
              tripal_report_error('rawpheno', TRIPAL_CRITICAL, 'Failed to load phenoypic data (job !id)', array('!id' => $job_id), array('print' => TRUE));
              exit(35);
            }
            
            // Get corresponding unit for this new trail based on cvterm_id
            // in chado.cvterm_relationship.
            $cv_unit = db_query("SELECT object_id FROM {chado.cvterm_relationship}
                                WHERE subject_id = :code LIMIT 1",
                                array(':code' => trim($type_id)))
              ->fetchField();
            $unit_id = $cv_unit;
            $unit = '';
            if (!$unit_id) {
              tripal_report_error(
                'rawpheno',
                TRIPAL_ERROR,
                'Uploading Phenoypic Data: Unable to find unit for Plant Measurement Type (New Trait; Term=!name; Type ID=!id) does\'t exist.',
                array('!name' => $cv_name, '!id' => $type_id),
                array('print' => TRUE)
              );
              $TRANSACTION->rollback();
              tripal_report_error('rawpheno', TRIPAL_CRITICAL, 'Failed to load phenoypic data (job !id)', array('!id' => $job_id), array('print' => TRUE));
              exit(36);
            }
          }
          else {
            // NOT NEW
            // Column header is not a new trait.
            // Remove unit from the header.
            if (in_array($cell_colheader, $arr_multiheaders)) {
              $cv_name = trim(str_replace(array(' cm', ' count'), '', $cell_colheader));
            }
            else {
              $cv_name = trim(preg_replace('/\([^)]+\)/', '', $cell_colheader));
            }
          
            $cv_name = str_replace(' ', '_', $cv_name);

            $type = tripal_get_cvterm(array('name' => $cv_name, 'cv_id' => array('name' => 'phenotype_measurement_types')));
            $type_id = $type->cvterm_id;
            if (!$type_id) {
              tripal_report_error(
                'rawpheno',
                TRIPAL_ERROR,
                'Uploading Phenoypic Data: Plant Measurement Type (Term=!name; Header=!colheader) does\'t exist.',
                array('!name' => $cv_name, '!colheader' => $cell_colheader),
                array('print' => TRUE)
              );
              $TRANSACTION->rollback();
              tripal_report_error('rawpheno', TRIPAL_CRITICAL, 'Failed to load phenoypic data (job !id)', array('!id' => $job_id), array('print' => TRUE));
              exit(37);
            }
            
            // Extract unit from the column header the cell is under.
            // If not unit, default to text unit
            $u = rawpheno_function_unit($cell_colheader);
            // Column header does not contain unit, use text as default
            $cv_unit = tripal_get_cvterm(array('name' => $u, 'cv_id' => array('name' => 'phenotype_measurement_units')));
            $unit_id = $cv_unit->cvterm_id;
            $unit = $u;
            if (!$unit_id) {
              tripal_report_error(
                'rawpheno',
                TRIPAL_ERROR,
                'Uploading Phenoypic Data: Unable to find unit for Plant Measurement Type (Term=!name; Type ID=!id; Unit=!unit) does\'t exist.',
                array('!name' => $cv_name, '!id' => $type_id, '!unit' => $unit),
                array('print' => TRUE)
              );
              $TRANSACTION->rollback();
              tripal_report_error('rawpheno', TRIPAL_CRITICAL, 'Failed to load phenoypic data (job !id)', array('!id' => $job_id), array('print' => TRUE));
              exit(37);
            }

          }
        
          // Determine if cell requires scale member code.
          // When unit is scale, find code equivalent in pheno_scale_member table.  
          if ($unit == 'scale') {
            // Get pheno scale member code 
            $cvalue_id = db_query("SELECT member_id FROM {pheno_scale_member} 
                                   WHERE code = :code LIMIT 1", 
                                   array(':code' => trim($cell_entry)))
              ->fetchField(); 
            if (!$cvalue_id) {
              tripal_report_error(
                'rawpheno',
                TRIPAL_WARNING,
                'Uploading Phenoypic Data: Unable to find scale id for Plant Measurement Type (Term=!name; Type ID=!id; Scale Value=!scale) does\'t exist.',
                array('!name' => $cv_name, '!id' => $type_id, '!scale' => $cell_entry),
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
          //
        
          // Insert trait only when type_id and unit_id are not null.
          if (isset($type_id) && isset($unit_id)) {
            $temp = db_insert('pheno_measurements')
              ->fields(array('plant_id'  => $pheno_plantid, 
                             'type_id'   => $type_id, 
                             'unit_id'   => $unit_id, 
                             'cvalue_id' => $cvalue_id,
                             'value'     => $cell_entry,
                             'modified'  => date("D M d, Y h:i:s a", time())))
              ->execute();
            if (!$temp) {
              tripal_report_error(
                'rawpheno',
                TRIPAL_ERROR,
                'Uploading Phenoypic Data: Unable to insert measurement. Values=@values.',
                array('@values' => print_r(array('plant_id'  => $pheno_plantid, 
                             'type_id'   => $type_id, 
                             'unit_id'   => $unit_id, 
                             'cvalue_id' => $cvalue_id,
                             'value'     => $cell_entry,
                             'modified'  => date("D M d, Y h:i:s a", time())),TRUE)),
                array('print' => TRUE)
              );
              $TRANSACTION->rollback();
              tripal_report_error('rawpheno', TRIPAL_CRITICAL, 'Failed to load phenoypic data (job !id)', array('!id' => $job_id), array('print' => TRUE));
              exit(38);
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
 */
function rawpheno_validate_excel_file($file) {
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
 * @TODO
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
 * @TODO
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

  // Retrieve the header for the indicated file.
  rawpheno_add_parsing_libraries();
  $xls_obj = rawpheno_open_file($file);
  rawpheno_change_sheet($xls_obj, 'measurements');
  // Note: we use the foreach here 
  // because the library documentation doesn't have a single fetch function.
  foreach ($xls_obj as $xls_headers) { break; }
  
  // Array to hold epected column headers.
  $expected_headers = rawpheno_function_headers('expected');
  
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