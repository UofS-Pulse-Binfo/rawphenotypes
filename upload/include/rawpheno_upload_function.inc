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
  //$file - Contains the path to xls or xlsx file.
  
  //Function call libraries_load() base on the implementation
  //of hook_libraries_info() in rawpheno.module.
  $xls_lib = libraries_load('spreadsheet_reader');
  //Library path information returned will be used
  //to include individual library files required.
  $lib_path = $xls_lib['path']; 

  //Include parser library. PLS DO NOT ALTER ORDER!!!
  //To stop parser from auto formatting date to MM/DD/YY,
  //suggest a new date format YYYY-mm-dd in:
  //  line 678 in excel_reader2.php
  //  line 835 in SpreadsheetReader_XLSX.php
  include_once $lib_path . 'php-excel-reader/excel_reader2.php';
  include_once $lib_path . 'SpreadsheetReader_XLSX.php';
  include_once $lib_path . 'SpreadsheetReader.php';  

  //Determine the stage requesting to read excel file.
  if (gettype($file) == 'string') {
    //STAGE 03
    //Read the file uploaded in stage 01. This condition
    //is for reading spreadsheet on stage 03, which the
    //absolute path to excel file uploaded to the server
    //in stage 01 - upload file.
    $xls_file = $file;
    $xls_extension = pathinfo($file, PATHINFO_EXTENSION);
  }
  else {
    //STAGE 01
    //Read a temporary file in stage 01.
    $xls_file = drupal_realpath($file->uri);
    $xls_extension = pathinfo($file->filename, PATHINFO_EXTENSION);
  }
  
  //Extract the file extension of the spreadsheet.
  if ($xls_extension == 'xlsx') {
    //XLSX
    $xls_obj = new SpreadsheetReader_XLSX($xls_file);
  } 
  else {
    //XLS
    //PLS INCLUDE THIS FILE ONLY FOR XLS TYPE ONLY.
    include_once $lib_path . 'SpreadsheetReader_XLS.php';
    $xls_obj = new SpreadsheetReader_XLS($xls_file);
  }
  
  //Array to hold each rows found in submitted file.
  $arr_usercols = array();
  
  //Locate measurement sheet.
  $xls_sheets = $xls_obj->Sheets();
  foreach($xls_sheets as $sheet_key => $sheet_val) {
    $xls_obj->ChangeSheet($sheet_key);
    
    //Start saving rows when in measurements sheet.
    if (strtolower(trim($sheet_val)) == 'measurements') {
      //Begin extrating first 11 rows, which will contain
      //first, last or 10th row required in performing 
      //basic compliance test.
      
      //This increment variable $i is required since xls and xlsx
      //parsers assign array index differently. 
      //XLS starts at 1, while XLSX at 0;
      $i = 0;
      foreach($xls_obj as $row) {
        //Convert row into a string and check the length.
        //This will exclude empty rows.        
        if (strlen(trim(implode('', $row))) > 2) {
          //Save row into the array.
          $arr_usercols[] = $row;
        }
        
        //Exit loop when there is at least 11, or the the last
        //row if there is less than 11 rows.
        //If request to read file is from stage 03 - save to database,
        //all rows will be saved into array.
        if ($i == 11 && gettype($file) != 'string') {        
          break;
        }
        
        $i++;
      }
      
      //Skip other sheets when measurement sheet is found.
      break;  
    }     
  }
  
  //Return array.
  //array(array(row 0), array(row 1), ...)
  return $arr_usercols;
}

/**
 * Function to validate column headers.
 *
 * @param array $header
 * @return array
 */
function rawpheno_function_chkheader($header) {
  //$header - Contains column headers from the spreadsheet.
  
  //Array to hold all missing headers found.
  $missing_header = array();
  //Get array of expected column headers.
  $arr_header = rawpheno_function_header('expected');
  
  //Read each cell in header row.
  foreach($header as $h_val) {
    $h_temp = rawpheno_function_nonewline($h_val);
    
    //Compare each required column header to the column header in the spreadsheet.
    if (array_key_exists($h_temp, $arr_header) && !empty($h_val)) {
      //Each reequired column header in the array is set to false by default (not found),
      //if header is found - mark it 1 (found).
      $arr_header[$h_temp] = 1;
    }
  }
  
  //Save all column headers with value set to 0 (not found)
  //and report to user as missing column header.
  foreach($arr_header as $h => $f) {
    if ($f == 0) {
      //Trait not found.
      array_push($missing_header, $h);
    } 
  }    
  
  //Return array.
  //array(header 1, header 2, ...)
  return $missing_header;
}

/**
 * Function to validate rows or non-column headers.
 *
 * @param array $row
 * @param array $header
 * @return array
 */
function rawpheno_function_chkrow($row, $header) {
  //$row - Contains a row in the spreadsheet.
  //$header - Contains column header from the spreadsheet.

  //Array to hold required cells that are empty.
  $arr_cell = array();
  //Array to hold cells with value that do not match the unit.
  $arr_unit = array();
  //Array to hold cells with stock name that does not exist.
  $arr_stock = array();
  //Get array of required column headers.
  $arr_required = rawpheno_function_id($header);
  
  //Read each cell in a data row.
  foreach($row as $r_key => $r_val) {
    //Determine if cell is required or not.
    if (array_key_exists($r_key, $arr_required)) {
		  //Requred columns plot, name, entry, rep and location.
		  if (empty($r_val)) {
		    //Required cell is empty.
		    $arr_cell[] = $arr_required[$r_key];
		  } 
		  elseif (strtolower($header[$r_key]) == 'name' &&
		          rawpheno_function_instock($r_val) <= 0) {
		    //Name with stock name that does not exist.
		    $arr_stock[] = 'Name: '.$r_val;
		  }
    }
    else {
      //If cell does not belong in required headers.
      //Match value to the unit.
      $r_header = $header[$r_key];
      //Extract the unit in the column header and test value against the unit.
      $r_unit = rawpheno_function_unit($r_header);
      
      if (rawpheno_function_chkunit($r_unit, $r_val) == 0 && !empty($r_val)) {
        //Does not match - create an error message about this cell.
        $arr_unit[] = $r_header .' - ('.$r_val.') not a valid ('.$r_unit.')';
      }
    }
  }
 
 //Return array.
 //array of empty required cells,
 //array of cells with value that do not match the unit,
 //array of cell with stock that does not exist.
 //array(array(), array(), array())
 return array($arr_cell, $arr_unit, $arr_stock); 
}

/**
 * Function to find the index of required headers from the submitted file.
 * Index will be used in case user alters order of column headers.
 *
 * @param array $headers
 * @return array
 */
function rawpheno_function_id($header) {
  //$header - Contains column header from the spreadsheet.
  
  //Array to hold required column headers.
  $arr_required = rawpheno_function_header('required');
  //Array to hold index of required column headers in the spreadsheet.
  $arr_index = array();
  
  //Find the actual index in the spreadsheet - in case the user
  //will try to alter the order of required column headers.
  foreach($arr_required as $r) {
    $i = array_search($r, array_map('strtolower', $header));   
    $arr_index[$i] = $r;
  }
  
  //Return array.
  //array('header 1' => 1, 'header 2' => 2, ...)
  return $arr_index;
}

/**
 * Function to remove new lines, extra spaces from a string.
 *
 * @param string $data
 * @return string
 */
function rawpheno_function_nonewline($data) {
  //$data - Contains the value of a cell
  
  $new_data = preg_replace('!\s+!', ' ', $data);

	//Return string.
	return strtolower(trim($new_data));  
}

/**
 * Function to create array of headers.
 *
 * @param string $request
 * @return array
 */
function rawpheno_function_header($request) {
  //$request - Contains the type of header required (expected, required)
  
  //Array of required headers.
  //column header => flag (1 found - 0 not found)
  $arr_expectedcols = array('plot' => 0, 
                            'entry' => 0,
                            'name' => 0,
                            'rep' => 0,
                            'location' => 0,
                            'planting date (date)' => 0,
                            '# of seeds planted (count)' => 0,
                            'days to emergence (days)' => 0,
                            '# of emerged plants (count)' => 0,
                            'days till 10% of plants have elongated tendrils (days)' => 0,
                            'days till 10% of plants have one open flower (r1; days)'  => 0,
                            '# nodes on primary stem at r1 (1st; count)' => 0,
                            '# nodes on primary stem at r1 (2nd; count)' => 0,
                            'days till 10% of plants have pods (r3; days)' => 0,
                            'days till 10% of plants have fully swollen pods (r5; days)' => 0,
                            'days till 10% of plants have 1/2 pods mature (r7; days)' => 0,
                            'r7 traits: lowest pod height (1st; cm)' => 0,
                            'r7 traits: lowest pod height (2nd; cm)' => 0,
                            'r7 traits: canopy height (1st; cm)' => 0,
                            'r7 traits: canopy height (2nd; cm)' => 0,
                            'days till harvest (days)' => 0,
                            'diseases present (y/n/?)' => 0,
                            'disease-specific comments' => 0,
                            'lodging (scale: 1-5) upright - lodged' => 0,
                            //NOTE THESE COLUMN HEADERS ARE HIDDEN
                            //Default value of # Peduncles is 20
                            'subset traits: # peduncles (count)' => 0,
                            'subset traits: # pods (count)' => 0,
                            'subset traits: # seeds (count)' => 0,
                            //
                            'straw biomass (g)' => 0,
                            'total seed mass (g)' => 0,
                            'total # of seeds (count)' => 0,
                            '100 seed mass (g)' => 0,
                            'comments' => 0);
    
                                
  //LIST OF REQUIRED HEADERS - this column headers must have value.
  $arr_requiredcols = array('entry', 'plot', 'name', 'rep', 'location');
  
  //Return array.
  //array(header 1, header 2, ....)
  return ( $request == 'expected' ) ? $arr_expectedcols : $arr_requiredcols;
}

/**
 * Function to extract the unit from the column header.
 *
 * @param string $header
 * @return string
 */
function rawpheno_function_unit($header) {
  //$header - Contains column header from the spreadsheet.
  
  $header = rawpheno_function_nonewline($header);
  $header = str_replace(array(';', '1st', '2nd', 'r1', 'r3', 'r5', 'r7', ': 1-5'), '', $header);
  preg_match("/.*\(([^)]*)\)/", $header, $match);
  
  //Return string.
  //Return unit found, or default to text if no unit found.
  return (isset($match[1])) ? trim($match[1]) : 'text';
}

/**
 * Function to match the values to the column header/trait unit.
 *
 * @param string $unit
 * @param string $cell_val
 * @return int
 */
function rawpheno_function_chkunit($unit, $value) {
  //$unit - Contains the unit from the column header.
  //$value - Contains the value of the cell.
  
  //Default to value matches the unit unless
  //proven otherwise.
  $pass = 1;
  
  switch($unit) {
    case 'date':
      //Date must be YYYY-MM-DD format .
      if (!empty($value) && preg_match('/[0-9]{4}-[0-9]{2}-[0-9]{2}/', $value) == 0) {
        $pass = 0;
      }
      break;
    
    case 'count':
    case 'days':
    case 'cm':
    case 'g':
      //Measurements - if it is numeric, then it must be greater than or equals to 0.
      if (!empty($value) && is_numeric($value) && $value < 0) {
        $pass = 0;      
      }
      break;
    
    case 'y/n/?':
      //Yes or No - if it is char, length is one, then check if y, n, ? (question mark).
      if (!empty($value) && strlen($value) == 1 && !in_array($value, array('y','n','?'))) {
        $pass = 0;
      }
      break;
    
    case 'scale':
      //Scale - if numeric, check if 1-5 range, inclusive.
      if (!empty($value) && is_numeric($value) && !($value >= 1 && $value < 6)) {
        $pass = 0;    
      } 
      break;
  }
  
  //Return 0 or 1.
  return $pass;
}

/**
 * Function to summarize all errors found.
 *
 * @param array $error
 * @param int $count
 * @return string
 */
function rawpheno_function_error($error, $count) {
  //$error - Contains an array of column headers or cell with errors.
  //$count - Contains the total number or rows.
  
  //Variable to hold errors.
  $txt_error = '';
  
  //Read each array.
  for($i = 0; $i < count($error); $i++) {
    //Determine if array has values.
    if (count($error[$i]) > 0) {
      if ($i == 0) {
        //TITLE FOR MISSING COLUMN HEADERS.
        $h = 'Missing required traits/column headers';
        $txt_error .= rawpheno_function_format($h, 'text'); 
      }
      
      //Read each error in the array.
      foreach($error[$i] as $c_key => $c_val) {
        if ($i == 0) {
          //DATA FOR MISSING COLUMN HEADERS.
          //List column headers that are missing.
          $txt_error .= rawpheno_function_format($c_val, 'cell');
        } 
        elseif (count($c_val) > 0){
          //TITLE FOR EMPTY CELL AND INVALID VALUE
          //Determine the type of error.
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
          
          //Determine which row has error.
          if ($i == 1) {
            $in_row = '* In First Row';
          }
          else {
            $in_row = ($count > 11) ? '* In 10th Row' : '* In Last Row';
          } 
          
          //Format the error and append it to text
          $txt_error .= rawpheno_function_format($in_row, 'text');
          
          //DATA FOR EMPTY CELL AND INVALID VALUE
          foreach($c_val as $c) {
            $txt_error .= rawpheno_function_format($c, 'cell');
          }
        } 
      }
    } 
  }
  
  //Return string.
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
function rawpheno_function_newheader($name, $header) {
  //$name - Contains the name of the variable to hold new headers found.
  //$header - Contains column header from the spreadsheet.

  //Array to hold epected column headers.
  $arr_expectedcols = rawpheno_function_header('expected');
  //Array to hold new column headers.
  $arr_newcols = array();
  
  if (count($header) > 0) {
    //Read each column header and compare against expected column headers.
    foreach($header as $key => $val) {
      $cell_value = rawpheno_function_nonewline($val);
      //Determine if column header exists in the expected column headers.
      if (!array_key_exists($cell_value, $arr_expectedcols) && !empty($val)) {
        //Not in expected column headers, save it as new header.
        array_push($arr_newcols, $cell_value);
      }
    }
  }
  
  //Store new headers found.
  //$arra_newcols = array('new header 1', 'new header 2', ...)
  variable_set($name, $arr_newcols);
}

/**
 * Function to reset variables.
 */
function rawpheno_function_resetvar() {
  if (variable_get('rawpheno_new_header')) {
    //Holds new column headers.
    variable_del('rawpheno_new_header');
  }
  
  if (variable_get('rawpheno_xls_file')) {
    //Holds the filename and location of spreadsheet.
    variable_del('rawpheno_xls_file');
  }
}

/**
 * Function to varify existence of stock name
 *
 * @param string $name
 * @return int
 */
function rawpheno_function_instock($name) {
  //$name - Contains the cell value under Name column header.
  $r = chado_select_record('stock', array('stock_id'), array('name' => $name));
 
  //Return int.
  //stock_id number.
  return (isset($r[0]->stock_id)) ? $r[0]->stock_id : 0;
}