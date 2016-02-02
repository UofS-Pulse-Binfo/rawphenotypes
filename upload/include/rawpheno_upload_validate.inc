<?php

/**
 * @file
 * Contains all implementations of hook_validate().
 */

/**
 * Validate traits review form.
 * 
 * @param type $form
 * @param type $form_state
 * @return type 
 */
function rawpheno_element_validate($form, &$form_state) {
  if (isset($form_state['stage']) && $form_state['stage'] == 'review') {
    return rawpheno_validate_review($form, $form_state);
  }
}

/**
 * Basic compliance test of xls or xlsx file.
 * 
 * @param type $form
 * @param type $form_state 
 */
function rawpheno_file_validate($file, $form_state) {
  //Variables containing spreadsheet file information.
  $xls_file = drupal_realpath($file->uri);
  $xls_extension = pathinfo($file->filename, PATHINFO_EXTENSION);  
  
  if(isset($file) && filesize($xls_file) && in_array($xls_extension, array('xls', 'xlsx'))) {
    //File exists and is valid xls or xlsx.
    //Array to hold first, last and 10th row.
    $arr_usercols = rawpheno_function_read($file);
    $user_colscount = count($arr_usercols);
    
    if( $user_colscount < 2 ) {
      //Missing measurement sheet or is empty worksheet.
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
      
      //Format all errors and display to user.
      $txt_error = rawpheno_function_error($xls_error, $user_colscount);
    }  
        
    //RETURN ERRORS
    if(strlen(trim($txt_error)) > 0) {
      //Errors found - display to user.
      $sheet_error = rawpheno_function_format('In Worksheet Measurements:', 'em');
      return '<pre id="stage01-window-error">'.$sheet_error.$txt_error.'</pre>';
    } 
    else {
      //No errors - save new header and path to file.
      variable_set('rawpheno_xls_file', drupal_realpath($file->destination));
      rawpheno_function_newheader('rawpheno_new_header', $arr_usercols[0]);
    }
    // 
  }  
}

/**
 * Implements hook_file_presave() to display file validation result.
 *
 * @param file $file
 */
function rawpheno_file_presave($file) {
  if (isset($file)) {
    //Read variable for new headers.
    $new_header = variable_get('rawpheno_new_header'); 
    
    if (count($new_header) > 0) {
      //New headers found in the variable must be described.
      $txt_header = rawpheno_function_format('In Worksheet Measurements:', 'em');
      $h = 'Found '.count($new_header).' additional column headers/traits that require more information.';
      $txt_header .= rawpheno_function_format($h, 'text');
    
      //Show user these new column headers found.
      foreach($new_header as $header) {
        $txt_header .= rawpheno_function_format($header, 'cell');
      }
    
      //Create a status window using this id.
      $id = 'stage01-window-review';
    }
    else {
      //No new column headers to describe.
      $txt_header = rawpheno_function_format('Spreadsheet passed basic compliance test.', 'em');
    
      //Create a status window using this id.
      $id = 'stage01-window-success';
    }
  
    //Report to user status or progress of state 01
    drupal_set_message('Spreadsheet is valid.<pre id="'.$id.'">'.$txt_header.'</pre>', 'status');
  }
}

/**
 * Validate entries in describe trait from.
 *
 * @param type $form
 * @param type $form_state 
 */
function rawpheno_validate_review($form, &$form_state) {
  //Include CSS - this is a fix to drupal not showing header information on hook_validate().
  $form['#attached']['css'] = array(drupal_get_path('module', 'rawpheno') . '/upload/css/rawpheno_upload_style.css');
 
  //Read new headers variable.
  $new_header = variable_get('rawpheno_new_header');
  
  if (isset($new_header) && count($new_header) > 0) {
    foreach($new_header as $k => $v) {
      //User supplied values to be validated. 
      $txt_header = $form_state['values']['txt_header_'.$k];
      $txt_def = $form_state['values']['txt_def_'.$k];
      $txt_unit = $form_state['values']['txt_unit_'.$k];
      $txt_describe = $form_state['values']['txtarea_describe_'.$k];
      
      //Validate only when user cheked checbox associated to trait.
      if( $form_state['values']['chk_'.$k] == 1 ) {
        if( empty($txt_def) ) {
          //Definition
          form_set_error('txt_def_'.$k, 'Header Definition is required in '.$txt_header);
        } 
      
        if( empty($txt_unit)) {
          //Unit
          form_set_error('txt_unit_'.$k, 'Header Unit is required in '.$txt_header);
        }
      
        if( empty($txt_describe) )  {
          //Description
          form_set_error('txtarea_describe_'.$k, 'Header Description is required in '.$txt_header);
        }
      }
    }
  } 
}