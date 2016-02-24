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
function rawpheno_load_spreadsheet($xls_file, $plantprop_headers, $job_id) {

  // Unserialize and remove formating for the headers that should be saved as 
  // plant properties rather than measurements.
  $plantprop_headers = unserialize($plantprop_headers);
  $plantprop_headers = array_map('rawpheno_function_delformat', $plantprop_headers);
  
  // Keep a record of which file we are loading in the logs.
  print "\nXLSX File: " . $xls_file . "\n";;
  
  // Array to hold all rows in the spreadsheet.
  $arr_xls_rows = rawpheno_function_read($xls_file);

  // Print a quick summary about the loading job.
  $num_rows = sizeof($arr_xls_rows);
  print "Number of lines read: " . $num_rows . "\n";
  print "Number of columns: " . sizeof($arr_xls_rows[0]) . "\n";
  
  // Find the index number of name header in the spreadsheet.
  $name_index = array_search('name', array_map('rawpheno_function_delformat', $arr_xls_rows[0]));
  
  // Array of column headers with first and second try.
  $arr_multiheaders = rawpheno_function_headers('multi-trial');
  $arr_multiheaders = array_map('rawpheno_function_delformat', $arr_multiheaders);

  // Read each row.
  print "\nNow parsing each row and saving it to the database...\nNumber of rows saved: ";
  for($i = 0; $i < count($arr_xls_rows); $i++) {
    
    // print update every 25 lines.
    if (($i % 25) == 0 AND $i !== 0) {
      print $i . "... ";
    }
    
    // Update job status every 5%
    $percent_complete = round(($i / $num_rows) * 100);
    if (($percent_complete % 5) == 0) {
      tripal_set_job_progress($job_id, $percent_complete);
    }
    
    // Rearrange array to make sure name is the first element,
    // and generate a plant_id number for the rest of the entries.
    $name_col = $arr_xls_rows[$i][$name_index];
    unset($arr_xls_rows[$i][$name_index]);
    array_unshift($arr_xls_rows[$i], $name_col);
      
    // Skip row with index 0, this is the column headers row.
    if ($i == 0) continue;
    
    // Read each row and each cell.
    // Each row will be an array where name is always the first element.
    foreach($arr_xls_rows[$i] as $cell_index => $cell_entry) {
      // Determine which table to insert a column header.
      if ($cell_index == 0) {
        // Name
        // First element in data array - Name cells
        // Name column header goes into pheno_plant.
        // Check if stock name exists.
        $stock_id = rawpheno_function_instock($cell_entry);
        
        // Determine if name has a stock id number.
        if (isset($stock_id) && $stock_id > 0) {
          // Plant_id for this row.
          $pheno_plantid = db_insert('pheno_plant')
            ->fields(array('stock_id' => $stock_id))
            ->execute();
        } else {
          // Next row if stock name does not exist.
          break;
        }
      } 
      else {
        // OTHER
        // The rest of the data array.
        // Get the column header of a cell.
        $cell_colheader = rawpheno_function_delformat($arr_xls_rows[0][$cell_index]);
        // Determine if column header is required
        if (in_array($cell_colheader, $plantprop_headers) && !empty($cell_colheader)) {
          // PLOT, ENTRY, REP and LOCATION
          // Cells containing column headers that are required.
          // Traits: plot, entry, rep, location into pheno_plantprop.
          $type = tripal_get_cvterm(array('name' => $cell_colheader, 'cv_id' => array('name' => 'phenotype_plant_property_types')));
          $type_id = $type->cvterm_id;
          
          // Ensure that cvterm_id is present before inserting to table
          if(isset($type_id)) {
            $tmp = db_insert('pheno_plantprop')
              ->fields(array('plant_id' => $pheno_plantid, 
                             'type_id'  => $type_id, 
                             'value'    => $cell_entry))
              ->execute(); 
          }
        } 
        elseif (!empty($cell_colheader)) {
          // THE REST OF THE COLUMN HEADERS
          // Everything else into pheno_measurements.
          // Determine if column header is New or not
          if (count($new_header) > 0 && in_array($cell_colheader, $new_header)) {
            // NEW
            // Test if column header of cell is a new trait.
            // Replace any extra spaces to underscore and create an exact match of
            // cvterm name inserted into cvterm table.
            $cv_name = str_replace(' ', '_', $cell_colheader);
            $type = tripal_get_cvterm(array('name' => $cv_name, 'cv_id' => array('name' => 'phenotype_measurement_types')));
            $type_id = $type->cvterm_id;
            
            // Get corresponding unit for this new trail based on cvterm_id
            // in chado.cvterm_relationship.
            $cv_unit = db_query("SELECT object_id FROM {chado.cvterm_relationship}
                                WHERE subject_id = :code LIMIT 1",
                                array(':code' => trim($type_id)))
              ->fetchField();
            $unit_id = $cv_unit;
            $unit = '';
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
          
            // Extract unit from the column header the cell is under.
            // If not unit, default to text unit
            $u = rawpheno_function_unit($cell_colheader);
            // Column header does not contain unit, use text as default
            $cv_unit = tripal_get_cvterm(array('name' => $u, 'cv_id' => array('name' => 'phenotype_measurement_units')));
            $unit_id = $cv_unit->cvterm_id;
            $unit = $u;
          }
          
          // Determine if cell requires scale member code.
          // When unit is scale, find code equivalent in pheno_scale_member table.  
          if ($unit == 'scale') {
            // Get pheno scale member code 
            $cvalue_id = db_query("SELECT member_id FROM {pheno_scale_member} 
                                   WHERE code = :code LIMIT 1", 
                                   array(':code' => trim($cell_entry)))
              ->fetchField(); 
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
          } 
        }
      }
    }
  }
  print "Complete.\n";
  
  print "\nUpdating the materialized view summarizing phenotypic data.\n";
  $mview_id = tripal_get_mview_id('rawpheno_rawdata_summary');
  if ($mview_id) tripal_populate_mview($mview_id);

}

/**
 * Function to read excel file (xls and xlsx).
 *
 * @param $file
 *   A string containing the path to a spreadsheet file.
 *
 * @return
 *   An array containing all data and column header rows in the spreadsheet.
 */
function rawpheno_function_read($file) {
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

  // Determine the stage requesting to read excel file.
  if (gettype($file) == 'string') {
    // STAGE 03
    // Read the file uploaded in stage 01. This condition
    // is for reading spreadsheet on stage 03, which the
    // absolute path to excel file uploaded to the server
    // in stage 01.
    $xls_file = $file;
    $xls_extension = pathinfo($file, PATHINFO_EXTENSION);
  }
  else {
    // STAGE 01
    // Read a temporary file in stage 01.
    $xls_file = drupal_realpath($file->uri);
    $xls_extension = pathinfo($file->filename, PATHINFO_EXTENSION);
  }
  
  // Extract the file extension of the spreadsheet.
  if ($xls_extension == 'xlsx') {
    // XLSX
    $xls_obj = new SpreadsheetReader_XLSX($xls_file);
  } 
  else {
    // XLS
    // PLS INCLUDE THIS FILE ONLY FOR XLS TYPE.
    include_once $lib_path . 'SpreadsheetReader_XLS.php';
    $xls_obj = new SpreadsheetReader_XLS($xls_file);
  }
  
  // Array to hold each row found in the spreadsheet.
  $arr_xls_rows = array();
  
  // Locate measurement sheet.
  $xls_sheets = $xls_obj->Sheets();
  foreach($xls_sheets as $sheet_key => $sheet_value) {
    $xls_obj->ChangeSheet($sheet_key);

    // Start saving rows when in measurements sheet.
    if (rawpheno_function_delformat($sheet_value) == 'measurements') {
      // Begin saving first 11 rows, which will contain
      // first, last or 10th row required in performing 
      // basic compliance test.

      // This increment variable $i is required since xls and xlsx
      // parsers assign array index differently. 
      // XLS starts at 1, while XLSX at 0;
      $i = 0;
      foreach($xls_obj as $row) {
        // Convert row into a string and check the length.
        // This will exclude empty rows.        
        if (strlen(trim(implode('', $row))) > 2) {
          // Save row into the array.
          $arr_xls_rows[] = $row;
        }

        // Exit loop when there is at least 11 rows, or all rows when there is less than 11 rows.
        // If request to read file is from stage 03, all rows will be saved into the array.
        if ($i == 11 && gettype($file) != 'string') {        
          break;
        }
        
        $i++;
      }
      
      // Skip other sheets when measurement sheet is found.
      break;  
    }     
  }

  return $arr_xls_rows;
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
 * Function to verify all expected column headers exist in the spreadsheet.
 *
 * @param $xls_headers
 *   An array containing all column headers in the spreadsheet.
 *  
 * @return
 *   An array containing all column headers not found in the spreadsheet.
 */
function rawpheno_function_chkheader($xls_headers) {
  // Array to hold all missing headers found.
  $arr_missing_headers = array();
  // Get array of expected column headers.
  $arr_expected_headers = rawpheno_function_headers('expected');
  //Remove any formatting in each column headers.
  $xls_headers = array_map('rawpheno_function_delformat', $xls_headers);
  
  // Read each expected column headers and compare it against
  // the column headers from the spreadsheet.
  foreach($arr_expected_headers as $header) {
    $temp_header = rawpheno_function_delformat($header);
      
    if (!in_array($temp_header, $xls_headers) && !empty($header)) {
      // An expected column header is not found. Save it.
      array_push($arr_missing_headers, $header);
    }
  }
  
  return $arr_missing_headers;
}

/**
 * Function to validate non column header rows.
 *
 * @param $xls_row
 *   An array containing a non column header row from the spreadsheet.    
 * @param $xls_headers
 *   An array containing all column headers in the spreadsheet.  
 *
 * @return
 *   An array containing all cells with error.
 */
function rawpheno_function_chkrow($xls_row, $xls_headers) {
  // Array to hold required cells that are empty.
  $arr_cell = array();
  // Array to hold cells with value that do not match the unit.
  $arr_unit = array();
  // Array to hold cells with stock name that does not exist.
  $arr_stock = array();
  // Array to hold index number of required column headers.
  $arr_required_id = rawpheno_function_id($xls_headers);
  
  //Read each cell in non-column header row.
  foreach($xls_row as $index => $value) {
    // Determine if cell is required or not.
    if (array_key_exists($index, $arr_required_id)) {
		  // Required - Cell belongs to either plot, name, entry, rep and location column.
		  if (empty($value)) {
		    //Required cell is empty.
		    $arr_cell[] = $arr_required_id[$index];
		  } 
		  elseif (rawpheno_function_delformat($xls_headers[$index]) == 'name' && rawpheno_function_instock($value) <= 0) {
		    // Name with stock name that does not exist.
		    $arr_stock[] = 'Name: ' . $value;
		  }
    }
    else {
      // Not required.
      // Match value to the unit.
      // Extract the unit in the column header and test the value against the unit.
      $unit = rawpheno_function_unit($xls_headers[$index]);

      if (rawpheno_function_chkunit($unit, $value) == 0 && !empty($value)) {
        // Does not match - create an error message about this cell.
        $arr_unit[] = $xls_headers[$index] . ' - ('. $value. ') not a valid (' . $unit . ')';
      }
    }
  }
 
 return array($arr_cell, $arr_unit, $arr_stock); 
}

/**
 * Function to find the index of required headers from the spreadsheet.
 * This will help locate required column headers in case user alters the order of columns.
 *
 * @param $xls_headers
 *   An array containing all column headers in the spreadsheet.  
 *
 * @return
 *   An array containing index numbers of each required column headers in the spreadsheet. 
 */
function rawpheno_function_id($xls_headers) {  
  // Array to hold required column headers.
  // plot, entry, name, rep, location
  $arr_required_headers = rawpheno_function_headers('required');
  // Array to hold index number of required column headers.
  $arr_required_id = array();
  // Remove any formatting in each column headers.
  $xls_headers = array_map('rawpheno_function_delformat', $xls_headers);
    
  // Find the actual index number of column header in the spreadsheet.
  foreach($arr_required_headers as $header) {
    $temp_header = rawpheno_function_delformat($header);
    $index = array_search($temp_header, $xls_headers);   
    $arr_required_id[$index] = $header;
  }
  
  return $arr_required_id;
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
 * Function to validate a cell has a value that matches the unit
 * in the column header.
 *
 * @param $xls_unit
 *   A string containing the unit extracted from the column header.
 * @param $xls_cell_value
 *   Contains the value of the cell.
 *
 * @return 
 *   Boolean true = 1 or false = 0.
 */
function rawpheno_function_chkunit($xls_unit, $xls_cell_value) {
  // All values match the unit by default. Unless proven otherwise.
  $is_valid = 1;
  
  switch($xls_unit) {
    case 'date':
      // Date must be YYYY-MM-DD format.
      if (!empty($xls_cell_value) && preg_match('/^[0-9]{4}-[0-9]{2}-[0-9]{2}/', $xls_cell_value) == 0) {
        $is_valid = 0;
      }
      break;
    
    case 'count':
    case 'days':
    case 'cm':
    case 'g':
      // Measurements - if it is numeric, then it must be greater than or equals to 0.
      if (!empty($xls_cell_value) && is_numeric($xls_cell_value) && $xls_cell_value < 0) {
        $is_valid = 0;      
      }
      break;
    
    case 'y/n/?':
      // Yes or No - if it is char, length is one, then check if y, n, ? (question mark).
      if (!empty($xls_cell_value) && strlen($xls_cell_value) == 1 && !in_array($xls_cell_value, array('y','n','?'))) {
        $is_valid = 0;
      }
      break;
    
    case 'scale':
      // Scale - if numeric, check if 1-5 range, inclusive.
      if (!empty($xls_cell_value) && is_numeric($xls_cell_value) && ($xls_cell_value >= 6 || $xls_cell_value < 1)) {
        $is_valid = 0;    
      } 
      break;
  }
  
  return $is_valid;
}

/**
 * Function to summarize all errors found. Each error is converted to
 * html form and displayed to the user.
 *
 * @param $xls_errors
 *   An array containing all errors found in the spreadsheet.
 * @param $row_count
 *   An integer containing the total number of rows parsed.
 *
 * @return
 *   A string containing all errors.
 */
function rawpheno_function_error($xls_errors, $row_count) {
  // Variable to hold errors after each error in the array
  // is converted into html.
  $txt_errors = '';
  
  // Read each array.
  for($i = 0; $i < count($xls_errors); $i++) {
    //Determine if array column header or data row.
    if (count($xls_errors[$i]) > 0) {
      if ($i == 0) {
        // Column header row.
        // TITLE FOR MISSING COLUMN HEADERS.
        $h = 'Missing required traits/column headers';
        $txt_errors .= rawpheno_function_format($h, 'text');
        // DATA FOR MISSING COLUMN HEADERS.
        foreach($xls_errors[$i] as $value) {
          // List column headers that are missing.
          $txt_errors .= rawpheno_function_format($value, 'cell');
        }
      } 
      else {
        // Data rows.
        // TITLE FOR CELLS WITH ERROR.
        // Determine which row has error.
        if ($i == 1) {
          $in_row = '* In First Row';
        }
        else {
          $in_row = ($row_count > 10) ? '* In 10th Row' : '* In Last Row';
        }

        foreach($xls_errors[$i] as $key => $value) {
          if (count($value) > 0) {  
            // Determine the type of error.
            if ($key == 0) {
              $error_type = 'Required cells have no data.';
            }
            elseif ($key == 1) {
              $error_type = 'Data that do not match the unit.';
            }
            elseif ($key == 2) {
              $error_type = 'Stock name does not exist.';
            }
          
            $txt_errors .= rawpheno_function_format($error_type, 'text');  
            $txt_errors .= rawpheno_function_format($in_row, 'text'); 
          
            // DATA FOR CELLS WITH ERROR.
            foreach($value as $cell_value) {
              $txt_errors .= rawpheno_function_format($cell_value, 'cell');
            }
          }
        }
      } 
    }    
  }
  
  return (strlen(trim($txt_errors)) > 0) ? $txt_errors : null;
}

/**
 * Function to format errors in html.
 *
 * @param $text
 *   A string containing a text information to format.
 * @param $class
 *   A string containing the css name. 
 *
 * @return
 *   A string containing the text wrapped in html with class
 *   attribute equals to the specified class name.
 */
function rawpheno_function_format($text, $class) {
  $css_class = 'error-message-'.$class;
  $text = ($class == 'cell') ? ucwords($text) : $text;
  
  return '<span class="'.$css_class.'">'.$text.'</span>'; 
}

/**
 * Function to find new headers.
 *
 * @param $var_name
 *   An string containing the name of the variable to hold new headers found.
 * @param $xls_headers
 *   An array containing all column headers in the spreadsheet.
 */
function rawpheno_function_newheader($var_name, $xls_headers) {
  // Array to hold epected column headers.
  $arr_expected_headers = rawpheno_function_headers('expected');
  // Remove any formatting in each column headers.
  $arr_expected_headers = array_map('rawpheno_function_delformat', $arr_expected_headers);
  // Array to hold new column headers.
  $arr_new_headers = array();
  
  if (count($xls_headers) > 0) {
    // Read each column header and compare against expected column headers.
    foreach($xls_headers as $value) {
      $temp_value = rawpheno_function_delformat($value);
      // Determine if column header exists in the expected column headers.
      if (!in_array($temp_value, $arr_expected_headers) && !empty($value)) {
        // Not in expected column headers, save it as new header.
        array_push($arr_new_headers, $value);
      }
    }
  }

  variable_set($var_name, $arr_new_headers);
}

/**
 * Function to reset variables.
 */
function rawpheno_function_resetvar() {
  if (variable_get('rawpheno_new_header')) {
    // Holds new column headers.
    variable_del('rawpheno_new_header');
  }
  
  if (variable_get('rawpheno_xls_file')) {
    // Holds the filename and location of spreadsheet.
    variable_del('rawpheno_xls_file');
  }
}

/**
 * Function to verify existence of stock name
 *
 * @param $xls_name
 *   A string containing the name of the stock to look up.  
 *
 * @return
 *   An interger containing the stock id number of the stock name.
 */
function rawpheno_function_instock($xls_name) {
  $stock = chado_select_record('stock', array('stock_id'), array('name' => $xls_name));
  return (isset($stock[0]->stock_id)) ? $stock[0]->stock_id : 0;
}