<?php

/**
 * @file
 * implements hook validate in phenotypes/upload page
 */

/**
 * validate traits review form.
 * 
 * @param type $form
 * @param type $form_state
 * @return type 
 */
function rawpheno_element_validate($form, &$form_state) {
  //validate describe additional traits
  //file upload validation is handled by file_validate hook function
  if (isset($form_state['stage']) && $form_state['stage'] == 'review') {
    return rawpheno_validate_review($form, $form_state);
  }
}

/**
 * basic compliance test - xls or xlsx file
 * 
 * @param type $form
 * @param type $form_state 
 */
function rawpheno_file_validate($file, $form_state) {
  $xls_file = drupal_realpath($file->uri);
  $xls_extension = pathinfo($file->filename, PATHINFO_EXTENSION);  
  if(isset($file) && filesize($xls_file) && in_array($xls_extension, array('xls', 'xlsx'))) {
    //file exists and is valid xls or xlsx 
    $arr_usercols = rawpheno_function_read($file);
    $user_colscount = count($arr_usercols);
    
    if( $user_colscount < 2 ) {
      //missing measurement sheet or is empty worksheet
      $h = 'No data to process or Measurements sheet is missing';
      $txt_error = rawpheno_function_format($h, 'em');
      $txt_error .= 'Please check spreadsheet for missing or blank sheet and try again.';
    }
    else {
      //VALIDATE HEADER & ROWS
      $xls_error = array();
      //validate header
      //$arr_usercols[0] - COLUMN HEADERS
      //$arr_usercols[1] - FIRST ROW
      $xls_error[0] = rawpheno_function_chkheader($arr_usercols[0]);
      
      //VALIDATE 1ST ROW - (first row, $headers);
      $xls_error[1] = rawpheno_function_chkrow($arr_usercols[1], $arr_usercols[0]);
      
      //VALIDATE LAST/10th ROW - (first row, $headers);
      if ($user_colscount > 2) {
        //get index of last or 10th row
        $i = ($user_colscount < 11) ? $user_colscount - 1 : 10;
        $xls_error[2] = rawpheno_function_chkrow($arr_usercols[$i], $arr_usercols[0]);
      }
      
      //all errors
      $txt_error = rawpheno_function_error($xls_error, $user_colscount);
    }  
        
    //RETURN ERRORS
    if(strlen(trim($txt_error)) > 0) {
      //errors found
      $sheet_error = rawpheno_function_format('In Worksheet Measurements:', 'em');
      return '<pre id="stage01-window-error">'.$sheet_error.$txt_error.'</pre>';
    } 
    else {
      //save new header and path to file in the server
      variable_set('rawpheno_xls_file', drupal_realpath($file->destination));
      rawpheno_function_newheader('rawpheno_new_header', $arr_usercols[0]);
    }
    // 
  }  
}

/**
 * hook file presave
 * display file validation result
 *
 * @param file $file
 */
function rawpheno_file_presave($file) {
  if (isset($file)) {
    $new_header = variable_get('rawpheno_new_header'); 
    if (count($new_header) > 0) {
      $txt_header = rawpheno_function_format('In Worksheet Measurements:', 'em');
      $h = 'Found '.count($new_header).' additional column headers/traits that require more information.';
      $txt_header .= rawpheno_function_format($h, 'text');
    
      //list all new column headers
      foreach($new_header as $header) {
        $txt_header .= rawpheno_function_format($header, 'cell');
      }
    
      //div window id
      $id = 'stage01-window-review';
    }
    else {
      $txt_header = rawpheno_function_format('Spreadsheet passed basic compliance test.', 'em');
    
      //div window id
      $id = 'stage01-window-success';
    }
  
    drupal_set_message('Spreadsheet is valid.<pre id="'.$id.'">'.$txt_header.'</pre>', 'status');
  }
}

/**
 * validate review new traits form
 *
 * @param type $form
 * @param type $form_state 
 */
function rawpheno_validate_review($form, &$form_state) {
  //include css
  $form['#attached']['css'] = array(drupal_get_path('module', 'rawpheno') . '/upload/css/rawpheno_upload_style.css');
  //new headers
  $new_header = variable_get('rawpheno_new_header');
  
  if (isset($new_header) && count($new_header) > 0) {
    foreach($new_header as $k => $v) {
      //text fields - all required
      $txt_header = $form_state['values']['txt_header_'.$k];
      $txt_def = $form_state['values']['txt_def_'.$k];
      $txt_unit = $form_state['values']['txt_unit_'.$k];
      $txt_describe = $form_state['values']['txtarea_describe_'.$k];
      
      //check only if trait is selected/checked
      if( $form_state['values']['chk_'.$k] == 1 ) {
        if( empty($txt_def) ) {
          //definition
          form_set_error('txt_def_'.$k, 'Header Definition is required in '.$txt_header);
        } 
      
        if( empty($txt_unit)) {
          //unit
          form_set_error('txt_unit_'.$k, 'Header Unit is required in '.$txt_header);
        }
      
        if( empty($txt_describe) )  {
          //description
          form_set_error('txtarea_describe_'.$k, 'Header Description is required in '.$txt_header);
        }
      }
    }
  } 
}