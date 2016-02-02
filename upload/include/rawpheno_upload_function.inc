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
  //Function call libraries_load() base on implementation of 
  //hook_libraries_info().
  $xls_lib = libraries_load('spreadsheet_reader');
  //Library path information returned which will be used
  //to include individual library files required.
  $lib_path = $xls_lib['path']; 

  //Include parser library. PLS DO NOT ALTER ORDER!!!
  //To stop parser from auto formatting date.
  //  line 678 in excel_reader2.php
  //  line 835 in SpreadsheetReader_XLSX.php
  //system date format: YYYY-mm-dd
  include_once $lib_path . 'php-excel-reader/excel_reader2.php';
  include_once $lib_path . 'SpreadsheetReader_XLSX.php';
  include_once $lib_path . 'SpreadsheetReader.php';  

  if (gettype($file) == 'string') {
    //Read the file uploaded in stage 01. This condition
    //is for reading spreadsheet on stage 03.
    $xls_file = $file;
    $xls_extension = pathinfo($file, PATHINFO_EXTENSION);
  }
  else {
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
    //PLS INCLUDE THIS ONLY FOR XLS FILE TYPE ONLY
    include_once $lib_path . 'SpreadsheetReader_XLS.php';
    $xls_obj = new SpreadsheetReader_XLS($xls_file);
  }
  
  //Array to hold each rows found in submitted file
  $arr_usercols = array();
  
  //Locate measurement sheet where all data for processing
  //is encoded.
  $xls_sheets = $xls_obj->Sheets();
  foreach($xls_sheets as $sheet_key => $sheet_val) {
    $xls_obj->ChangeSheet($sheet_key);
    if (strtolower(trim($sheet_val)) == 'measurements') {
      //Begin extrating first 11 rows which will contain
      //first, last or 10th row required in performing 
      //basic compliance test.
      $i = 0;
      foreach($xls_obj as $key => $rows) {       
        if (strlen(trim(implode('', $rows))) > 2) {
          //only rows with data in it
          $arr_usercols[] = $rows;
        }
        
        //Get the first 11 row for validation
        //0 header, 1st row and last or 10th row.
        //If request to read file is from stage 03 - then all
        //rows will be returned for saving to database.
        if ($i == 11 && gettype($file) != 'string') {        
          break;
        }
        
        $i++;
      }
      
      //Skip other sheets when measurement sheet is found.
      break;  
    }     
  }
  
  return $arr_usercols;
}

/**
 * Function to validate column headers.
 *
 * @param array $header
 * @return array
 */
function rawpheno_function_chkheader($header) {
  //Array to hold all missing headers found.
  $missing_header = array();
  //Get array of expected column headers
  $arr_header = rawpheno_function_header('expected');
  
  foreach($header as $h_val) {
    $h_temp = rawpheno_function_nonewline($h_val);
  
    if (array_key_exists($h_temp, $arr_header) && !empty($h_val)) {
      //Each reequired column header in the array is set to false by default (not found),
      //if header is found - mark it 1 (found)
      $arr_header[$h_temp] = 1;
    }
  }
  
  //Save all column headers with value set to 0 (not found)
  foreach($arr_header as $h => $f) {
    if ($f == 0) {
      //Trait not found.
      array_push($missing_header, $h);
    } 
  }    
  
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
  //Array to hold required cells that are empty.
  $arr_cell = array();
  //Array to hold cells with value that do not  match the unit.
  $arr_unit = array();
  //Array to hold cells with stock name that does not exist.
  $arr_stock = array();
  //Get array of required column headers.
  $arr_required = rawpheno_function_id($header);
  
  foreach($row as $r_key => $r_val) {
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
      //If cell does not belong to require headers.
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
 
 //Return array of empty required cells,
 //array of cells with value that do not match the unit,
 //array of cell with stock that does not exist.
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
  //Array to hold required column headers.
  $arr_required = rawpheno_function_header('required');
  //Array to hold index of required column headers
  //when located in the spreadsheet.
  $arr_index = array();
  
  //Find the actual index in the spreadsheet - in case the user
  //altered the order of required column headers
  foreach($arr_required as $r) {
    $i = array_search($r, array_map('strtolower', $header));   
    $arr_index[$i] = $r;
  }
  
  return $arr_index;
}

/**
 * Function to remove new lines, extra spaces from a string.
 *
 * @param string $data
 * @return string
 */
function rawpheno_function_nonewline($data) {
  $new_data = preg_replace('!\s+!', ' ', $data);

	return strtolower(trim($new_data));  
}

/**
 * Function to create array of headers.
 *
 * @param string $request
 * @return array
 */
function rawpheno_function_header($request) {
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
  
  return ( $request == 'expected' ) ? $arr_expectedcols : $arr_requiredcols;
}

/**
 * Function to extract the unit from the column header.
 *
 * @param string $header
 * @return string
 */
function rawpheno_function_unit($header) {
  $header = rawpheno_function_nonewline($header);
  $header = str_replace(array(';', '1st', '2nd', 'r1', 'r3', 'r5', 'r7', ': 1-5'), '', $header);
  preg_match("/.*\(([^)]*)\)/", $header, $match);
  
  //Return unit found, or default to text if unit found.
  return (isset($match[1])) ? trim($match[1]) : 'Text';
}

/**
 * Function to match the values to the column header/trait unit.
 *
 * @param string $unit
 * @param string $cell_val
 * @return int
 */
function rawpheno_function_chkunit($unit, $cell_val) {
  $pass = 1;
  
  switch( $unit ) {
    case 'date':
      //Date must be YYYY-MM-DD format .
      if (!empty($cell_val) && preg_match('/[0-9]{4}-[0-9]{2}-[0-9]{2}/', $cell_val) == 0) {
        $pass = 0;
      }
      break;
    
    case 'count':
    case 'days':
    case 'cm':
    case 'g':
      //Measurements - if it is numeric, then it must be greater than or equals to 0.
      if (!empty($cell_val) && is_numeric($cell_val) && $cell_val < 0) {
        $pass = 0;      
      }
      break;
    
    case 'y/n/?':
      //Yes or No - if it is char, length is one, then check if y, n, ? (question mark).
      if (!empty($cell_val) && strlen($cell_val) == 1 && !in_array($cell_val, array('y','n','?'))) {
        $pass = 0;
      }
      break;
    
    case 'scale':
      //Scale - if numeric, check if 1-5 range, inclusive.
      if (!empty($cell_val) && is_numeric($cell_val) && !($cell_val >= 1 && $cell_val < 6)) {
        $pass = 0;    
      } 
      break;
  }
  
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
  //Variable to hold errors.
  $txt_error = '';
  
  //Process array of errors passed.
  for($i = 0; $i < count($error); $i++) {
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
  $text = ($class == 'cell') ? ucwords($text) : $text;
  return '<span class="error-message-'.$class.'">'.$text.'</span>'; 
}

/**
 * Function to find new headers.
 *
 * @param string $name
 * @param array $header
 */
function rawpheno_function_newheader($name, $header) {
  //Array to hold epected column headers.
  $arr_expectedcols = rawpheno_function_header('expected');
  //Array to hold new column headers.
  $arr_newcols = array();
  
  if (count($header) > 0) {
    foreach($header as $key => $val) {
      $cell_valtemp = rawpheno_function_nonewline($val);
      if (!array_key_exists($cell_valtemp, $arr_expectedcols) && !empty($val)) {
        array_push($arr_newcols, $cell_valtemp);
      }
    }
  }
  
  //Store new headers found.
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
 $r = chado_select_record('stock', array('stock_id'), array('name' => $name));
 
 return (isset($r[0]->stock_id)) ? $r[0]->stock_id : 0;
}