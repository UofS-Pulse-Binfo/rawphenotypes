<?php

/**
 * @file
 * Functions required in processing a spreadsheet.
 */

/**
 * Load the Spreadsheet into the database. This should be executed as a Tripal Job.
 *
 * @param $project_id
 *   An integer containing the project id the spreadsheet, headers and data are specific to.
 * @param $arr_newheaders
 *   An array of columns detected in the spreadsheet that are user defined and not in the project header set.
 * @param $fid
 *   An integer containing the file id number assigned to a spreadsheet file during upload process.
 * @param $plantprop_headers
 *   An array of column headers of type plant property (Name, Plot, Entry, Rep and Location in this module).
 * @param $job_id
 *   The tripal job id.
 */
function rawpheno_load_spreadsheet($project_id, $arr_newheaders, $fid, $plantprop_headers, $job_id) {
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
    tripal_report_error('rawpheno', TRIPAL_CRITICAL, '[CODE 100] Failed to load phenoypic data (job !id)', array('!id' => $job_id), array('print' => TRUE));
    exit(1);
  }

  $xls_file = drupal_realpath($file->uri);

  // INFO:
  // Keep a record of which file we are loading in the logs.
  // print "\nXLSX File: " . $xls_file . "\n";

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
    tripal_report_error('rawpheno', TRIPAL_CRITICAL, '[CODE 101] Failed to load phenoypic data (job !id)', array('!id' => $job_id), array('print' => TRUE));
    exit(2);
  }

  // Change to the correct spreadsheet.
  rawpheno_change_sheet($xls_obj, 'measurements');

  // Compute the total number of rows parsed.
  $row_count = rawpheno_count_rows($xls_obj);

  // File to write progress status.
  $tmp = file_directory_temp();
  $filename = $tmp . '/' . 'job-progress' . $job_id . '.txt';

  // Start Transaction.
  $TRANSACTION = db_transaction();
  try {

    // Read each row.
    // INFO:
    // print "\nNow parsing each row and saving it to the database...\nNumber of rows saved: \n";
    $i = 0;

    // Each row in the spreadsheet.
    foreach ($xls_obj as $row) {
      // Create progress update by computing the number of rows saved in percent.
      // Write to file and progress bar API reads the content and pass it JSON generator
      // in file rawpheno.module function: rawpheno_upload_job_progress_json().
      // Echo to terminal.

      $percent = round(($i / $row_count) * 100);
      if ($percent % 10 == 0) {
        print $percent . '% complete...' . "\n";
      }

      // To file.
      file_unmanaged_save_data($percent, $filename, FILE_EXISTS_REPLACE);

      // HEADER!
      // This is the header.
      if ($i == 0) {
        $header = $row;

        // Find the index number of name header in the spreadsheet.
        $name_index = array_search('name', array_map('rawpheno_function_delformat', $header));
        if ($name_index === FALSE) {
          tripal_report_error(
            'rawpheno',
            TRIPAL_CRITICAL,
            'Uploading Phenoypic Data: Unable to determine the name column.',
            array(),
            array('print' => TRUE)
          );
          $TRANSACTION->rollback();
          tripal_report_error('rawpheno', TRIPAL_CRITICAL, '[CODE 102] Failed to load phenoypic data (job !id)', array('!id' => $job_id), array('print' => TRUE));
          exit(31);
        }

        $i++;
        continue;
      }

      // NEXT ROW IF EMPTY!
      // Don't continue processing if this row is empty.
      if (strlen(trim(implode('', $row))) <= 5) {
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
        $p_id = 0;

        // Test if stock was measured in the active project, if not, insert as a new record.
        // Otherwise, do more check (plot) to see if plant_id should be re-used.
        $sql = "
          SELECT plant_id
          FROM pheno_plant AS t1 INNER JOIN pheno_plant_project AS t2 USING(plant_id)
          WHERE t1.stock_id = :stock_id AND t2.project_id = :project_id LIMIT 1";

        $args = array(':stock_id' => $stock_id, ':project_id' => $project_id);
        $p = chado_query($sql, $args);

        if ($p->rowCount()) {
          // Found a stock record in the project. Do more test.
          // Array to hold plot headers.
          // Plot, Rep, Location, Planting Date (date)
          $arr_plot_cols = rawpheno_function_headers('plot');

          // Construct query string.
          // String : stock_id - plot - rep - location - year
          //        eg. 147-5-2-Saskatoon-2015
          $plot = $stock_id;

          // Given a row, construct the search string (format) above and use it to search if
          // the such combination matched any record in the database.
          foreach($arr_plot_cols as $plot_col) {
            $plot_col = rawpheno_function_delformat($plot_col);

            // Cell value of plot property header.
            $col_index = array_search(strtolower($plot_col), array_map('rawpheno_function_delformat', $header));
            $cell_val = trim($row[$col_index]);

            // If planting date - extract the year value.
            if ($plot_col == 'plantingdate(date)') {
              $y = explode('-', $cell_val);
              $cell_val = $y[0];
            }

            $plot .= '-' . $cell_val;
          }

          // Search the query string.
          $p_id = rawpheno_function_plot_exists($plot, $project_id);
        }


        if ($p_id) {
          // Plot found - re-use the plant_id.
          $pheno_plantid = $p_id;
          // INFO:
          // print 'FOUND PLOT: ' . $plot . ' [re-using plot id #' . $pheno_plantid . '] ~ ';
        }
        else {
          // Plot not found - insert as new row.
          $pheno_plantid = db_insert('pheno_plant')
            ->fields(array('stock_id' => $stock_id))
            ->execute();

          // Map this record/stock to a project.
          db_insert('pheno_plant_project')
            ->fields(array('project_id' => $project_id,
                           'plant_id' => $pheno_plantid))
            ->execute();

          // INFO:
          // print 'NEW STOCK: #' . $stock_id . ' [adding plot id #' . $pheno_plantid . '] ~ ';
        }
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
        tripal_report_error('rawpheno', TRIPAL_CRITICAL, '[CODE 103] Failed to load phenoypic data (job !id)', array('!id' => $job_id), array('print' => TRUE));
        exit(32);
      }

      // Read each row and each cell.
      // Each row will be an array where name is always the first element.
      foreach($row as $cell_index => $cell_entry) {
        // We don't want to insert empty data.
        // That said, while PHP thinks 0 is empty, we do not.
        if (!empty($cell_entry) OR (strval($cell_entry) === '0')) {

          // Get the column header of a cell.
          $cell_colheader = trim(str_replace(array("\n", "\r", "  "), ' ', $header[$cell_index]));

          // Determine if user wants to save this trait.
          if (count($arr_newheaders) > 0 AND array_key_exists($cell_colheader, $arr_newheaders)) {
            if ($arr_newheaders[$cell_colheader]['flag'] == 0) {
              // Skip this cell if it is a new column header and user does not want to save
              // this new trait;
              continue;
            }
            elseif ($arr_newheaders[$cell_colheader]['flag'] == 1) {
              // Get the cvterm name for this new header.
              $alt_name = $arr_newheaders[$cell_colheader]['alt_header'];
              $name = tripal_get_cvterm(array('cvterm_id' => $alt_name, 'cv_id' => array('name' => 'phenotype_measurement_types')));
              $cell_colheader = $name->name;
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
                tripal_report_error('rawpheno', TRIPAL_CRITICAL, '[CODE 104] Failed to load phenoypic data (job !id)', array('!id' => $job_id), array('print' => TRUE));
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
              tripal_report_error('rawpheno', TRIPAL_CRITICAL, '[CODE 105] Failed to load phenoypic data (job !id)', array('!id' => $job_id), array('print' => TRUE));
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
              tripal_report_error('rawpheno', TRIPAL_CRITICAL, '[CODE 106] Failed to load phenoypic data (job !id)', array('!id' => $job_id), array('print' => TRUE));
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
                array('!name' => $cell_colheader, '!id' => $type_id),
                array('print' => TRUE)
              );
              $TRANSACTION->rollback();
              tripal_report_error('rawpheno', TRIPAL_CRITICAL, '[CODE 107] Failed to load phenoypic data (job !id)', array('!id' => $job_id), array('print' => TRUE));
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
                tripal_report_error('rawpheno', TRIPAL_CRITICAL, '[CODE 108] Failed to load phenoypic data (job !id)', array('!id' => $job_id), array('print' => TRUE));
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
    tripal_report_error('rawpheno', TRIPAL_CRITICAL, '[CODE 109] Failed to load phenoypic data (job !id)', array('!id' => $job_id), array('print' => TRUE));
    exit(4);
  }

  unset($TRANSACTION); //Commit
  print "Upload complete.\n";

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
 * @param $source
 *   A string containing the source of the file upload - Upload Data or Backup File.
 *
 * @return
 *   An array containing the validation result from each validator.
 */
function rawpheno_validate_excel_file($file, $project_id, $source) {
  $status = array();

  // Process the validators to make them easier to use.
  // Specifically, sort them by their scope.
  $validators = array();
  $all_validators = module_invoke_all('rawpheno_validators');
  foreach($all_validators as $k => $v) {
    $validators[ $v['scope'] ][ $k ] = $v;
  }

  // Todo list.
  $all_scope_validators = array('project', 'file', 'all', 'header', 'subset');

  // Add the libraries needed to parse excel files.
  rawpheno_add_parsing_libraries();

  // Before performing any validation to the excel file. Ensure first that a project is selected.
  foreach ($validators['project'] as $prj_validator_name => $prj_validator) {
    if (isset($prj_validator['validation callback']) AND function_exists($prj_validator['validation callback'])) {
      $status[ $prj_validator_name ] = call_user_func($prj_validator['validation callback'], $project_id);

      // If returned false then halt validation.
      if ($status[ $prj_validator_name ] === FALSE) {
        // Fail the project and set the rest to TODO.
        $status[ $prj_validator_name ] = FALSE;

        // Todo the rest of validators.
        // Since this is project scope and it got falsed - remove the project.
        unset($all_scope_validators[0]);
        foreach($all_scope_validators as $v) {
          foreach($validators[ $v ] as $v_name => $validator) {
            $status[ $v_name ] = 'todo';
          }
        }

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
        // Fail the file and set the rest to TODO but set the project to passed
        // first since it is assumed that project validator returned a passed value.
        $status[ 'project_selected' ] = TRUE;
        $status[ $validator_name ] = FALSE;

        // Todo the rest of validators.
        // Since project is completed. skip this scope.
        unset($all_scope_validators[0]);
        foreach($all_scope_validators as $v) {
          foreach($validators[ $v ] as $v_name => $validator) {
            if ($status[ $v_name ] === TRUE) {
              $status[ $v_name ] = TRUE;
            }
            elseif ($status[ $v_name ] === FALSE) {
              $status[ $v_name ] = FALSE;
            }
            else {
              $status[ $v_name ] = 'todo';
            }
          }
        }

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
    if (strlen(trim(implode('', $row))) >= 5) {

      // VALIDATE THE HEADER.
      if ($i == 1) {
        // Save the header for later.
        $header = array();
        $new_header = array();
        // Checking plot value requires cell value in Planting Date (date) and Location.
        // Store index numbers of these two traits.
        $plot_req = array();

        $o = 0;
        foreach ($row as $r) {
          $without_format = rawpheno_function_delformat($r);
          $no_units = rawpheno_get_trait_name($r);
          $header[] = array(
            'no format' => $without_format,
            'original' => $r,
            'units' => rawpheno_function_unit($without_format),
            'no units' => $no_units,
          );

          // Store index number of Plot trait requirements.
          if (!isset($plot_req['planting date (date)']) && $without_format == 'planting date (date)') {
            $plot_req['planting date (date)'] = $o;
          }
          elseif (!isset($plot_req['location']) && $without_format == 'location') {
            $plot_req['location'] = $o;
          }

          $o++;
        }

        // Foreach validator with a scope of header, execute the validation callback & save the results.
        foreach($validators['header'] as $validator_name => $validator) {
          if (isset($validator['validation callback']) AND function_exists($validator['validation callback'])) {
            $result = call_user_func($validator['validation callback'], $header, $project_id);

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

                  // If column header is Plot, attach Plot validation requirement to
                  // $context array. The indexes will be used to fetch the cell value in context row.
                  if ($column_name == 'Plot') {
                    $context['plot_req'] = $plot_req;
                  }

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
          $check_limit_message = "We have only checked the first $i lines of your file. Please fix the errors reported below and then upload the fixed file.";

          if ($source == 'upload') {
            drupal_set_message($check_limit_message, 'error');
            return $status;
          }
          elseif ($source == 'backup') {
            return array('status' => $status, 'check_limit' => $check_limit_message);
          }
        }
      }
    }
  }

  // Make sure all validators are represented in status.
  // If they are not already then a failure wasn't recorded -thus they passed :-).
  foreach($all_validators as $validator_name => $validator) {
    if (!isset($status[$validator_name])) {
      $status[$validator_name] = TRUE;
    }
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
  // Get all the sheets in the workbook.
  $xls_sheets = $xls_obj->Sheets();

  // Locate the measurements sheet.
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
  //     0xe => "m/d/Y", to 0xe => "Y-m-d",
  //   line 834 in SpreadsheetReader_XLSX.php
  //     $Value = $Value -> format($Format['Code']); to $Value = $Value -> format('Y-m-d');
  //
  include_once $lib_path . 'php-excel-reader/excel_reader2.php';
  include_once $lib_path . 'SpreadsheetReader_XLSX.php';
  include_once $lib_path . 'SpreadsheetReader.php';

  if ($file_type == 'XLS') {
    // PLS INCLUDE THIS FILE ONLY FOR XLS TYPE.
    include_once $lib_path . 'SpreadsheetReader_XLS.php';
  }
}


/**
 * Function to remove all formatting from a cell value.
 *
 * @param $xls_cell_value
 *   Contains a value of a cell.
 * @return
 *   Contains a cell value with all formatting removed.
 */
function rawpheno_function_delformat($xls_cell_value) {
  // Remove any extra spaces, new lines, leading and trainling spaces
  // and covert the final result to lowercase.
  return trim(strtolower(preg_replace('!\s+!', '', $xls_cell_value)));
}


/**
 * Function to extract the unit from the column header.
 *
 * @param $xls_header_cell
 *   A string containing a column header.
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
 * Function to determine additional column headers in the spreadsheet. Additional column headers are
 * headers that are no part of the predefined headers set of the project.
 *
 * @param $file
 *   The full path to the excel file containing data.
 * @param $project_id
 *   Project id number the spreadsheet is specific to.
 * @return
 *    An array containing all additional column headers detected.
 */
function rawpheno_indicate_new_headers($file, $project_id) {
  // Retrieve the header for the indicated file.
  rawpheno_add_parsing_libraries();
  $xls_obj = rawpheno_open_file($file);
  rawpheno_change_sheet($xls_obj, 'measurements');

  // Note: we use the foreach here
  // because the library documentation doesn't have a single fetch function.
  foreach ($xls_obj as $xls_headers) { break; }

  // Array to hold epected column headers specific to a given project.
  $expected_headers = rawpheno_project_traits($project_id);

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
        $new_headers[] = $value;
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
 * Count all rows in a spreadsheet.
 *
 * @param $file
 *   The full path to the excel file containing data.
 * @return
 *   An integer value of the total rows.
 */
function rawpheno_count_rows($xls_obj) {
  // Row of 5 chars or more long is a row.
  $count_rows = 0;
  foreach ($xls_obj as $row) {
    if (strlen(implode('', $row)) > 5) {
      $count_rows++;
    }
  }

  // Less header row.
  return $count_rows - 1;
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

  // Query trait. Module stores unit in lowercase but user can use any case
  // in the spreadsheet. eg Planting Date (date) and Planting Date (Date).
  $sql = "SELECT t2.cvterm_id
          FROM {cv} AS t1 INNER JOIN {cvterm} AS t2 USING(cv_id)
          WHERE lower(t2.name) = :cvterm_name AND t1.name = :cv_name
          LIMIT 1";

  $args = array(':cvterm_name' => trim(strtolower($header)), ':cv_name' => 'phenotype_measurement_types');
  $type = chado_query($sql, $args)
    ->fetchObject();

  if ($type->cvterm_id) {
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
    $trait_id = rawpheno_get_trait_id($trait_name);
  }

  // First we try to get the unit through relationships since that avoids making assumptions.
  // in chado.cvterm_relationship.
  $sql = "SELECT rel.object_id as id, cvt.name as name
          FROM chado.cvterm_relationship rel LEFT JOIN chado.cvterm cvt ON cvt.cvterm_id = rel.object_id
          WHERE rel.subject_id = :trait LIMIT 1";

  $args = array(':trait' => $trait_id);
  $unit = db_query($sql, $args)
    ->fetchAll();

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
 * Get all the essential traits in a project.
 *
 * @param $project_id
 *   An integer containing the project ID number.
 * @return
 *   An array containing all essential traits in a project.
 */
function rawpheno_project_essential_traits($project_id) {
  if (isset($project_id) AND $project_id > 0) {
    // Get array of trait types
    $trait_type = rawpheno_function_trait_types();

    // Array to hold trait names.
    $arr_essential_traits = array();

    // Query essential traits in a project.
    $sql = "SELECT TRIM(t1.name) AS cvterm
            FROM {cvterm} AS t1 RIGHT JOIN pheno_project_cvterm AS t2 USING (cvterm_id)
            WHERE
              t2.project_id = :project_id
              AND t2.type IN (:essential)
            ORDER BY t1.name ASC";

    $args = array(':project_id' => $project_id, ':essential' => array($trait_type['type1'], $trait_type['type4']));
    $trait = chado_query($sql, $args);

    foreach($trait as $t) {
      $m = rawpheno_get_trait_name($t->cvterm);
      $arr_essential_traits[] = $m;
    }

    // Add Name column header to the traits returned.
    $arr_essential_traits[] = 'Name';

    return $arr_essential_traits;
  }
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
  if (isset($project_id) AND $project_id > 0) {
    // Get array of trait types
    $trait_type = rawpheno_function_trait_types();

    // Array to hold trait names.
    $arr_plantproperty_traits = array();

    // Query plant property traits in a project.
    $sql = "SELECT TRIM(t1.name) AS cvterm
            FROM {cvterm} AS t1 RIGHT JOIN pheno_project_cvterm AS t2 USING (cvterm_id)
            WHERE t2.project_id = :project_id AND t2.type = :plantproperty
            ORDER BY t1.name ASC";

    // traits of type plantproperty.
    $args = array(':project_id' => $project_id, ':plantproperty' => $trait_type['type4']);
    $trait = chado_query($sql, $args);

    foreach($trait as $t) {
      // Remove the trait rep and unit from the trait.
      $m = rawpheno_get_trait_name($t->cvterm);
      $arr_plantproperty_traits[] = $m;
    }

    return $arr_plantproperty_traits;
  }
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
  if (isset($project_id) AND $project_id > 0) {
    $arr_trait = array();

    // Query column headers in a project.
    $sql = "SELECT TRIM(t1.name) AS cvterm
            FROM {cvterm} AS t1 RIGHT JOIN pheno_project_cvterm AS t2 USING (cvterm_id)
            WHERE t2.project_id = :project_id
            ORDER BY t1.name ASC";

    $args = array(':project_id' => $project_id);
    $trait = chado_query($sql, $args);

    foreach($trait as $t) {
      $arr_trait[] = $t->cvterm;
    }

    // Add Name column header to the traits returned.
    $arr_trait[] = 'Name';

    return $arr_trait;
  }
}


/**
 * Function to fetch information about a unit (Describe method) when available.
 *
 * @param $cvterm_id
 *   An integer containing the cvterm id of a trait.
 */
function rawpheno_function_cvterm_properties($cvterm_id) {
  // Narrow the search to cvterm of type measurement units.
  $cv_unit = tripal_get_cv(array('name' => 'phenotype_measurement_units'));

  // In form state 2, describe header, user has the opportunity to describe the unit (Describe method field).
  // This information is stored in cvterm relationship together with the cvterm id of the unit as the subject_id
  // and cvterm_id of the header as the object_id. Given a header cvterm id, get the subject id and use it to
  // get the information required from cvtermprop table.
  $sql = "SELECT subject_id FROM {cvterm_relationship} WHERE object_id = :cvterm_id AND type_id = :cv_unit LIMIT 1";
  $args = array(':cvterm_id' => $cvterm_id, ':cv_unit' => $cv_unit->cv_id);

  $d = chado_query($sql, $args)
    ->fetchField();

  $sql = "SELECT value FROM {cvtermprop} WHERE type_id = :cv_unit AND cvterm_id = :cvterm_id LIMIT 1";
  $args = array(':cv_unit' => $cv_unit->cv_id, ':cvterm_id' => $d);

  $d = chado_query($sql, $args);

  if ($d->rowCount() == 1) {
    return $d->fetchField();
  }
  else {
    return 'Describe the method used not available';
  }
}
