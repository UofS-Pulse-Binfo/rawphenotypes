<?php

/**
 * @file
 * functions reqiured in file upload.
 */

/**
 * read excel file (xls and xlsx)
 *
 * @param object $file
 * @return array
 */
function rawpheno_function_read($file) {
  //include parser library. DO NOT ALTER ORDER!!!
  include_once RAWPHENO_PATH . '/upload/include/spreadsheet-reader/php-excel-reader/excel_reader2.php';
  include_once RAWPHENO_PATH . '/upload/include/spreadsheet-reader/SpreadsheetReader_XLSX.php';
  include_once RAWPHENO_PATH . '/upload/include/spreadsheet-reader/SpreadsheetReader.php';  
  
  if (gettype($file) == 'string') {
    //read actual file in the server in stage 03
    $xls_file = $file;
    $xls_extension = pathinfo($file, PATHINFO_EXTENSION);
  }
  else {
    //read temp upload file in stage 01
    $xls_file = drupal_realpath($file->uri);
    $xls_extension = pathinfo($file->filename, PATHINFO_EXTENSION);
  }
  
  //extract the file extension of the spreadsheet
  if ($xls_extension == 'xlsx') {
    $xls_obj = new SpreadsheetReader_XLSX($xls_file);
  } 
  else {
    //needs this library exclusive to xls file version 
    include_once RAWPHENO_PATH . '/upload/include/spreadsheet-reader/SpreadsheetReader_XLS.php';
    $xls_obj = new SpreadsheetReader_XLS($xls_file);
  }
  
  //holds cols/rows found in submitted file
  $arr_usercols = array();
  
  //load it to an array - exclude empty rows  
  $xls_sheets = $xls_obj->Sheets();
  foreach($xls_sheets as $sheet_key => $sheet_val) {
    $xls_obj->ChangeSheet($sheet_key);
    if (strtolower(trim($sheet_val)) == 'measurements') {
      //measurements sheet found
      $i = 0;
      foreach($xls_obj as $key => $rows) {       
        if (strlen(trim(implode('', $rows))) > 2) {
          //only rows with data in it
          $arr_usercols[] = $rows;
        }
        
        //get the first 11 row for validation
        //0 header, 1st row and last or 10th row
        if ($i == 11 && gettype($file) != 'string') {        
          break;
        }
        
        $i++;
      }
      
      //exit loop once measurement sheet is found
      break;  
    }     
  }
  
  return $arr_usercols;
}

/**
 * validate column headers
 *
 * @param array $header
 * @return array
 */
function rawpheno_function_chkheader($header) {
  $missing_header = array();
  //get array of expected column headers
  $arr_header = rawpheno_function_header('expected');
  foreach($header as $h_val) {
    $h_temp = rawpheno_function_nonewline($h_val);
  
    if (array_key_exists($h_temp, $arr_header) && !empty($h_val)) {
      //required column header array set to false by default (not found)
      //if header is found - mark it 1 (found)
      $arr_header[$h_temp] = 1;
    }
  }
  
  foreach($arr_header as $h => $f) {
    if ($f == 0) {
      //trait not found
      array_push($missing_header, $h);
    } 
  }    
  
  return $missing_header;
}

/**
 * validate rows/ non column headers
 *
 * @param array $row
 * @param array $header
 * @return array
 */
function rawpheno_function_chkrow($row, $header) {
  //holds error for empty cells
  $arr_cell = array();
  //holds error for value not matching the unit
  $arr_unit = array();
  $arr_required = rawpheno_function_id($header);
  foreach($row as $r_key => $r_val) {
    if (array_key_exists($r_key, $arr_required) && empty($r_val)) {
		  //requred columns plot, name, entry, rep and location
		  $arr_cell[] = $arr_required[$r_key];
    }
    else {
      //match value to the unit
      //get the column header of the cell
      $r_header = $header[$r_key];
      //extract the unit
      $r_unit = rawpheno_function_unit($r_header);
      //test unit and cell value
      if (rawpheno_function_chkunit($r_unit, $r_val) == 0 && !empty($r_val)) {
        //does not match - create an error message about this cell
        $arr_unit[] = $r_header .' - ('.$r_val.') not a valid ('.$r_unit.')';
      }
    }
  }
  
 return array($arr_cell, $arr_unit); 
}

/**
 * find the index of required headers from the submitted file
 * in case user alters order of column headers
 *
 * @param array $headers
 * @return array
 */
function rawpheno_function_id($header) {
  //holds list of required column headers
  $arr_required = rawpheno_function_header('required');
  //holds index of required column headers
  $arr_index = array();
  
  //find the actual index in the spreadsheet - in case the user
  //altered the order of required column headers
  foreach($arr_required as $r) {
    $i = array_search($r, array_map('strtolower', $header));   
    $arr_index[$i] = $r;
  }
  
  return $arr_index;
}

/**
 * remove new lines, extra spaces from a string
 *
 * @param string $data
 * @return string
 */
function rawpheno_function_nonewline($data) {
  $new_data = preg_replace('!\s+!', ' ', $data);

	return strtolower(trim($new_data));  
}

/**
 * return array of headers
 *
 * @param string $request
 * @return array
 */
function rawpheno_function_header($request) {
  //array of required headers
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
    
                                
  //LIST OF REQUIRED HEADERS - this column headers must have value
  $arr_requiredcols = array('entry', 'plot', 'name', 'rep', 'location');
  
  return ( $request == 'expected' ) ? $arr_expectedcols : $arr_requiredcols;
}

/**
 * extract the unit from the column header
 *
 * @param string $header
 * @return string
 */
function rawpheno_function_unit($header) {
  $header = rawpheno_function_nonewline($header);
  $header = str_replace(array(';', '1st', '2nd', 'r1', 'r3', 'r5', 'r7', ': 1-5'), '', $header);
  preg_match("/.*\(([^)]*)\)/", $header, $match);
  
  if (isset($match[1])) {
    return trim($match[1]);
  } else {
    //no unit - will default to text
    return 'text';
  }
}

/**
 * match the entry to the column header/trait unit
 *
 * @param string $unit
 * @param string $cell_val
 * @return int
 */
function rawpheno_function_chkunit($unit, $cell_val) {
  $pass = 1;
  
  switch( $unit ) {
    case 'date':
      //at least 3 characters 
      if (!empty($cell_val) && preg_match('/[0-9]{2}-[0-9]{2}-[0-9]{4}/', $cell_val) == 0) {
        $pass = 0;
      }
      
      break;
    
    case 'count':
    case 'days':
    case 'cm':
    case 'g':
      //if it is numeric, then it must be greater than 0
      if (!empty($cell_val) && is_numeric($cell_val) && $cell_val < 0) {
        $pass = 0;      
      }
      
      break;
    
    case 'y/n/?':
      //if it is char, length is one, then check if y, n, ?
      if (!empty($cell_val) && strlen($cell_val) == 1 && !in_array($cell_val, array('y','n','?'))) {
        $pass = 0;
      }
      
      break;
    
    case 'scale':
      //if numeric, check if 1-5 range, inclusive
      if (!empty($cell_val) &&  is_numeric($cell_val) &&  !($cell_val > 0 && $cell_val < 6)) {
        $pass = 0;    
      } 
      
      break;
  }
  
  return $pass;
}

/**
 * report all errors found to user
 *
 * @param array $error
 * @param int $count
 * @return string
 */
function rawpheno_function_error($error, $count) {
  $txt_error = '';
  
  for($i = 0; $i < count($error); $i++) {
    if (count($error[$i]) > 0) {
      //process array if it contains errors
      if ($i == 0) {
        //HEADER FOR MISSING COLUMN HEADERS
        $h = 'Missing required traits/column headers';
        $txt_error .= rawpheno_function_format($h, 'text'); 
      }
      
      //read each error
      foreach($error[$i] as $c_key => $c_val) {
        if ($i == 0) {
          //DATA FOR MISSING COLUMN HEADERS
          //show column headers that are missing
          $txt_error .= rawpheno_function_format($c_val, 'cell');
        } 
        elseif (count($c_val) > 0){
          //HEADER FOR EMPTY CELL AND INVALID VALUE
          //what type of error (empty cell/ invalid value)
          $err_type = ( $c_key == 0 ) ? 'Required cells have no data' : 
                                        'Data that do not match the unit';
          $txt_error .= rawpheno_function_format($err_type, 'text'); 
          
          //which row
          if ($i == 1) {
            $in_row = '* In First Row';
          }
          else {
            $in_row = ($count > 11) ? '* In 10th Row' : '* In Last Row';
          } 
          
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
 * wrap errors in html tag
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
 * find new headers and make it available to stages
 *
 * @param string $name
 * @param array $header
 */
function rawpheno_function_newheader($name, $header) {
  $arr_expectedcols = rawpheno_function_header('expected');
  $arr_newcols = array();
  if (count($header) > 0) {
    foreach($header as $key => $val) {
      $cell_valtemp = rawpheno_function_nonewline($val);
      if (!array_key_exists($cell_valtemp, $arr_expectedcols) && !empty($val)) {
        array_push($arr_newcols, $cell_valtemp);
      }
    }
  }
  
  //store new headers
  variable_set($name, $arr_newcols);
}

/**
 * clear variables
 */
function rawpheno_function_resetvar() {
  if (variable_get(RAWPHENO_UPLOAD_NEW_HEADER)) {
    //hold new headers
    variable_del(RAWPHENO_UPLOAD_NEW_HEADER);
  }
  
  if (variable_get(RAWPHENO_UPLOAD_XLS_FILE)) {
    //holds the filename and location of spreadsheet uploaded
    variable_del(RAWPHENO_UPLOAD_XLS_FILE);
  }
}