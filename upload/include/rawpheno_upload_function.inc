<?php

/**
 * @file
 * Functions required in processing a spreadsheet.
 */

/**
 * Function to read excel file (xls and xlsx).
 *
 * @param string/object $file
 * @return array
 */
function rawpheno_function_read($file) {
  // $file - Contains the path to xls or xlsx file.
  
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
  //   line 835 in SpreadsheetReader_XLSX.php
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
  
  // Array to hold each rows found in submitted file.
  $arr_usercols = array();
  
  // Locate measurement sheet.
  $xls_sheets = $xls_obj->Sheets();
  foreach($xls_sheets as $sheet_key => $sheet_val) {
    $xls_obj->ChangeSheet($sheet_key);
    
    // Start saving rows when in measurements sheet.
    if (strtolower(trim($sheet_val)) == 'measurements') {
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
          $arr_usercols[] = $row;
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
  
  // Return array.
  // array(array(row 0), array(row 1), ...)
  return $arr_usercols;
}

/**
 * Function to validate column headers.
 *
 * @param array $xls_header
 * @return array
 */
function rawpheno_function_chkheader($xls_header) {
  // $xls_header - Contains column header row from the spreadsheet.
  
  // Array to hold all missing headers found.
  $missing_header = array();
  // Get array of expected column headers.
  $expected_header = rawpheno_function_headers('expected');
  
  // Read each expected column headers and compare it against
  // the column headers from the spreadsheet.
  foreach($expected_header as $header) {
    $h = rawpheno_function_nonewline($header);
    $temp_header = strtolower($h);
      
    if (!in_array($temp_header, $xls_header) && !empty($header)) {
      // An expected column header is not found. Save it.
      array_push($missing_header, $header);
    }
  }
  
  // Return array.
  // array(header 1, header 2, ...)
  return $missing_header;
}

/**
 * Function to validate non-column headers.
 *
 * @param array $xls_row
 * @param array $xls_header
 * @return array
 */
function rawpheno_function_chkrow($xls_row, $xls_header) {
  // $xls_row - Contains a non-column header row from the spreadsheet.
  // $xls_header - Contains column header row from the spreadsheet.

  // Array to hold required cells that are empty.
  $arr_cell = array();
  // Array to hold cells with value that do not match the unit.
  $arr_unit = array();
  // Array to hold cells with stock name that does not exist.
  $arr_stock = array();
  // Array to hold index number of required column headers.
  $arr_required_id = rawpheno_function_id($xls_header);
  
  //Read each cell in non-column header row.
  foreach($xls_row as $index => $value) {
    //Determine if cell is required or not.
    if (array_key_exists($index, $arr_required_id)) {
		  //Required - Cell belongs to either plot, name, entry, rep and location column.
		  if (empty($value)) {
		    //Required cell is empty.
		    $arr_cell[] = $arr_required_id[$index];
		  } 
		  elseif (strtolower($xls_header[$index]) == 'name' && rawpheno_function_instock($value) <= 0) {
		    //Name with stock name that does not exist.
		    $arr_stock[] = 'Name: '.$value;
		  }
    }
    else {
      //Not required.
      //Match value to the unit.
      //Extract the unit in the column header and test the value against the unit.
      $unit = rawpheno_function_unit($xls_header[$index]);
      
      if (rawpheno_function_chkunit($unit, $value) == 0 && !empty($value)) {
        //Does not match - create an error message about this cell.
        $arr_unit[] = $xls_header[$index] .' - ('.$value.') not a valid ('.$unit.')';
      }
    }
  }
 
 // Return array.
 // array of empty required cells,
 // array of cells with value that do not match the unit,
 // array of cell with stock that does not exist.
 // array(array(), array(), array())
 return array($arr_cell, $arr_unit, $arr_stock); 
}

/**
 * Function to find the index of required headers from the spreadsheet.
 * This will help locate required column headers in case user alters the order of columns.
 *
 * @param array $xls_header
 * @return array
 */
function rawpheno_function_id($xls_header) {
  // $xls_header - Contains column header row from the spreadsheet.
  
  // Array to hold required column headers.
  // plot, entry, name, rep, location
  $required_header = rawpheno_function_headers('required');
  // Array to hold index number of required column headers.
  $arr_required_id = array();
  
  // Find the actual index number in the spreadsheet.
  foreach($required_header as $header) {
    $temp_header = strtolower($header);
    $index = array_search($temp_header, array_map('strtolower', $xls_header));   
    $arr_required_id[$index] = $header;
  }
  
  // Return array.
  // array('plot' => index #, 'entry' => index #, ...)
  return $arr_required_id;
}

/**
 * Function to remove new lines, extra spaces from a string.
 *
 * @param string $xls_cell_value
 * @return string
 */
function rawpheno_function_nonewline($xls_cell_value) {
  // $xls_cell_value - Contains the value of a cell
  
  // Remove extra spaces and new lines.
  // Return string.
  return preg_replace('!\s+!', ' ', $xls_cell_value);
}

/**
 * Function to extract the unit from the column header.
 *
 * @param string $header
 * @return string
 */
function rawpheno_function_unit($xls_header_cell) {
  // $xls_header - Contains the value of a column header cell from the spreadsheet.
  
  $temp_value = rawpheno_function_nonewline($xls_header_cell);
  $cell_value = str_replace(array(';', '1st', '2nd', 'r1', 'r3', 'r5', 'r7', ': 1-5'), '', $temp_value);
  preg_match("/.*\(([^)]*)\)/", $cell_value, $match);
  
  // Return string.
  // Return unit found, or default to text if no unit.
  return (isset($match[1])) ? trim($match[1]) : 'text';
}

/**
 * Function to match a value to unit.
 *
 * @param string $unit
 * @param string $value
 * @return int
 */
function rawpheno_function_chkunit($unit, $value) {
  //$unit - Contains the unit extracted from the column header
  //$value - Contains the value of the cell.
  
  //All values match the unit by default. Unless proven otherwise.
  $is_valid = 1;
  
  switch($unit) {
    case 'date':
      // Date must be YYYY-MM-DD format.
      if (!empty($value) && preg_match('/[0-9]{4}-[0-9]{2}-[0-9]{2}/', $value) == 0) {
        $is_valid = 0;
      }
      break;
    
    case 'count':
    case 'days':
    case 'cm':
    case 'g':
      // Measurements - if it is numeric, then it must be greater than or equals to 0.
      if (!empty($value) && is_numeric($value) && $value < 0) {
        $is_valid = 0;      
      }
      break;
    
    case 'y/n/?':
      // Yes or No - if it is char, length is one, then check if y, n, ? (question mark).
      if (!empty($value) && strlen($value) == 1 && !in_array($value, array('y','n','?'))) {
        $is_valid = 0;
      }
      break;
    
    case 'scale':
      // Scale - if numeric, check if 1-5 range, inclusive.
      if (!empty($value) && is_numeric($value) && !($value >= 1 && $value < 6)) {
        $is_valid = 0;    
      } 
      break;
  }
  
  // Return 0 or 1.
  return $is_valid;
}

/**
 * Function to summarize all errors found.
 *
 * @param array $arr_error
 * @param int $row_count
 * @return string
 */
function rawpheno_function_error($arr_error, $row_count) {
  // $arr_error - Contains an array or errors found in the spreadsheet.
  // $row_count - Contains the total number or rows.
  
  // Variable to hold errors.
  $txt_error = '';
  
  // Read each array.
  for($i = 0; $i < count($arr_error); $i++) {
    // Determine if array has values.
    if (count($arr_error[$i]) > 0) {
      if ($i == 0) {
        // TITLE FOR MISSING COLUMN HEADERS.
        $h = 'Missing required traits/column headers';
        $txt_error .= rawpheno_function_format($h, 'text'); 
      }
      
      // Read each error in the array.
      foreach($error[$i] as $c_key => $c_val) {
        if ($i == 0) {
          // DATA FOR MISSING COLUMN HEADERS.
          // List column headers that are missing.
          $txt_error .= rawpheno_function_format($c_val, 'cell');
        } 
        elseif (count($c_val) > 0){
          // TITLE FOR EMPTY CELL AND INVALID VALUE
          // Determine the type of error.
          if ($c_key == 0) {
            $err_type = 'Required cells have no data.';
          }
          elseif ($c_key == 1) {
            $err_type = 'Data that do not match the unit.';
          } 
          elseif ($c_key == 2) {
            $err_type = 'Stock name does not exist.';
          }
                                        
          $txt_error .= rawpheno_function_format($err_type, 'text'); 
          
          // Determine which row has error.
          if ($i == 1) {
            $in_row = '* In First Row';
          }
          else {
            $in_row = ($count > 11) ? '* In 10th Row' : '* In Last Row';
          } 
          
          // Format the error and append it to text
          $txt_error .= rawpheno_function_format($in_row, 'text');
          
          // DATA FOR EMPTY CELL AND INVALID VALUE
          foreach($c_val as $c) {
            $txt_error .= rawpheno_function_format($c, 'cell');
          }
        } 
      }
    } 
  }
  
  // Return string.
  return (strlen(trim($txt_error)) > 0) ? $txt_error : null;
}

/**
 * Function to format errors in html.
 *
 * @param string $text
 * @param string $class 
 * @return string
 */
function rawpheno_function_format($text, $class) {
  //$text - Contains string/text to format
  //$class - Contains css name (text or cell).
  
  $css_class = 'error-message-'.$class;
  $text = ($class == 'cell') ? ucwords($text) : $text;
  
  //Return string.
  return '<span class="'.$css_class.'">'.$text.'</span>'; 
}

/**
 * Function to find new headers.
 *
 * @param string $name
 * @param array $header
 */
function rawpheno_function_newheader($var_name, $xls_header) {
  // $var_name - Contains the name of the variable to hold new headers found.
  // $xls_header - Contains the value of a column header cell from the spreadsheet.

  // Array to hold epected column headers.
  $expected_header = array_map('strtolower', rawpheno_function_headers('expected'));
  // Array to hold new column headers.
  $new_header = array();
  
  if (count($xls_header) > 0) {
    // Read each column header and compare against expected column headers.
    foreach($xls_header as $value) {
      $temp_value = rawpheno_function_nonewline($value);
      // Determine if column header exists in the expected column headers.
      if (!in_array($temp_value, $expected_header) && !empty($value)) {
        // Not in expected column headers, save it as new header.
        array_push($new_header, $value);
      }
    }
  }
  
  // Store new headers found.
  // $new_header = array('new header 1', 'new header 2', ...)
  variable_set($name, $arr_newcols);
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
 * Function to varify existence of stock name
 *
 * @param string $xls_name
 * @return int
 */
function rawpheno_function_instock($xls_name) {
  //$xls_name - Contains the cell value under Name column header.
  $stock = chado_select_record('stock', array('stock_id'), array('name' => $name));
 
  //Return int.
  //stock_id number.
  return (isset($stock[0]->stock_id)) ? $stock[0]->stock_id : 0;
}