<?php

/**
 * @file
 * Contains the upload raw phenotypic data form.
 */

// Additional Helper functions
module_load_include('inc', 'rawpheno', 'include/rawpheno.upload.helpers');

/**
 * Master form constructor which calls an individual form for each stage.
 * 
 * @see rawpheno_element_validate()
 * @see rawpheno_element_submit()
 */
function rawpheno_upload_form_master($form, &$form_state) {  

  // If the stage is not set then default to the first stage (i.e. 'check' )
  if (!isset($form_state['stage'])) {
    $form_state['stage'] = 'check';
  }

  // If a job_id was provide in the URL then the user wants information
  // on a prvious bulk loading job. Thus we should show them the last step.
  if (isset($form_state['build_info']['args'][0])) {
    $form_state['stage'] = 'save';
  }

  // Display some basic information/guidance.
  $form['help']['help_text'] = array(
    '#type' => 'markup',
    '#markup' => '<p>'
    . t('This form will guide you through uploading your raw phenotypic data. Your data should be in a <em>Microsoft Excel Workbook (XLSX) following the format described on the <a href="@instructions-page" target="_blank">Instructions Page</a></em>. In the second step we ask that you <em>describe any additional phenotypes</em>. Finally, the spreadsheet is saved to KnowPulse, at which point the <em>phenotypic data is available through <a href="@summary-page" target="_blank">summaries</a> and <a href="@download-page" target="_blank">downloads</a></em>.',
      array(
        '@instructions-page' => url('phenotypes/raw/instructions'),
        '@summary-page' => url('phenotypes/raw'),
        '@download-page' => url('phenotypes/raw/download'),
      ))
    .'</p>'
  );
  
  // Add the stage tracker/header.
  $form = rawpheno_get_header($form, $form_state);

  // Add the next button for all but the last step.
  if ($form_state['stage'] != 'save') {
    $form['next_step'] = array(
      '#type' => 'submit',
      '#value' => 'Next Step',
      '#weight' => 100
    );
  }
  
  // Get the directory path to rawpheno module.
  $path = drupal_get_path('module', 'rawpheno') . '/theme/';
  
  // Load corresponding form together with JavaScript and CSS.
  switch($form_state['stage']) {
    case 'check':
      // Stage 01 - upload and check spreadsheet.
      $form['#attached']['css'] = array($path . 'css/rawpheno.upload.stage01.css');
      $form['#attached']['js']  = array($path . 'js/rawpheno.upload.stage01.js');
      
      $form = rawpheno_upload_form_stage_check($form, $form_state);
      break;

    case 'review':
      // Stage 02 - describe form (only when there is additional trait).
      $form['#attached']['css'] = array($path . 'css/rawpheno.upload.stage02.css');
      
      $form = rawpheno_upload_form_stage_review($form, $form_state);
      break;
	 
    case 'save':
      // Stage 03 - save to databse and success page.
      $form['#attached']['css'] = array($path . 'css/rawpheno.upload.stage03.css');

      $form = rawpheno_upload_form_stage_save($form, $form_state);
      break;
  }
  
  return $form;
}

/**
 * Function callback: Construct form for Stage 01.
 *
 * Stage 01 form allows user to upload data collection spreadsheet and
 * perform basic compliance test.
 */
function rawpheno_upload_form_stage_check($form, &$form_state) {
  
  // Create an instance of DragNDrop Upload.
  // SETTINGS:
  //   file_upload_max_size: max file size allowed
  //   upload_location: destination of file
  //   upload_event: manual - show an upload button or auto - uploads after drag drop
  //   upload_validators: allowed file extensions
  //   upload_button_text: label of upload button
  //   droppable_area_text: text in drop area
  //   progress_indicator: none, throbber or bar
  //   progress_message: message to display while processing
  $form['dnd'] = array(
    '#type' => 'dragndrop_upload',
    '#file_upload_max_size' => '10M',
    '#upload_location' => 'public://',
    '#upload_event' => 'auto',
    '#upload_validators' => array('file_validate_extensions' => array('xlsx xls')),
    '#droppable_area_text' => t('Drag Phenotype Data Collection file here'),
    '#progress_indicator' => 'throbber',
    '#progress_message' => '',
  );

  return $form;  
}

/**
 * Function callback: Construct form for Stage 02.
 *
 * Stage 02 form allows user to describe and save a additional trait/s found
 * in the spreadsheet submitted in Stage 01.
 *
 * Assuming the file uploaded properly, we have access to an excel file and need to
 * ensure that all traits have been described. If there are any that haven't then
 * we need to ask the the user to define them now.
 */
function rawpheno_upload_form_stage_review(&$form, &$form_state) {
  $new_header = array();
  
  // First step, determine which headers/traits need to be described.
  if (isset($form_state['multistep_values']['fid'])) {
    $file = file_load($form_state['multistep_values']['fid']);
    if ($file) {
      $new_header = rawpheno_indicate_new_headers($file);
      $form_state['multistep_values']['new_headers'] = $new_header;
    }
    else {
      drupal_set_message('Unable to access your file. Please try uploading again.', 'error');
    }
  }
  else {
    drupal_set_message('We have no record of your uploaded file. Please try uploading it again.', 'error');
  }
  
  // If we were unable to access the file then don't let them proceed.
  if (!isset($file) OR empty($file)) {
    $form['notice'] = array(
      '#type' => 'markup',
      '#markup' => '<div class="messages error">'
        . t('Unable to access uploaded file. Please attempt to upload your file again <a href="@upload-page">on the previous page</a>. If the problem persists then contact the administrator.',
            array('@upload-page' => url('phenotypes/raw/upload')))
        . '</div>',
    );
    
    unset($form['next_step']);
    
    return $form;
  }
  
  // If there are no new headers then they don't have to do anything.
  if (empty($new_header)) {
    $form['notice'] = array(
      '#type' => 'markup',
      '#markup' => '<div class="messages status">No new traits were detected in the spreadsheet. Please click "Next Step".</div>',
    );
    
    return $form;
  }

  // Otherwise, we need a form!
  // Main fieldset container for form elements.
  $form['xls_review_fldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Check the traits that you want to describe and save'),
  );

  foreach($new_header as $i => $k) {
    if (isset($k) && !empty($k)) {
    
      // Fieldset to contain a set of trait and corresponding form elements.
      $form['xls_review_fldset']['fldset_' . $i] = array(
        '#type' => 'fieldset',
        '#title' => '',
        '#prefix' => '<span id="trait-description-' . $i . '">',
        '#suffix' => '</span>',
      );

      // All fields are required.
      // CHECKBOX to let user select a trait to describe and save.
      $form['xls_review_fldset']['fldset_' . $i]['chk_' . $i] = array(
        '#type' => 'checkbox',
        '#title' => t(ucwords($k)),
        '#ajax' => array(
          'callback' => 'ajax_rawpheno_upload_form_step2_expand_trait_callback',
          'wrapper' => 'trait-description-' . $i,
          'effect' => 'fade',
          'trait_index' => $i,
        )
      );
        
      // If the checkbox is checked then show the fields we want described.
      if (isset($form_state['values']['chk_' . $i]) AND ($form_state['values']['chk_' . $i] == TRUE)) {

        // TERM NAME/TRAIT/HEADER
        $form['xls_review_fldset']['fldset_' . $i]['txt_header_' . $i] = array(
          '#type' => 'hidden',
          '#value' => $k,
        ); 
          // These fields are hidden by default - show only when checked.
          // TERM DEFINITION
          $form['xls_review_fldset']['fldset_' . $i]['txt_def_' . $i] = array(
            '#type' => 'textfield',
            '#title' => t('Definition'),
            '#required' => TRUE,
            '#description' => t('A human-readable text definition'),
            '#maxlength' => 150,
            '#prefix' => '<div class="container-form" id="div-edit-chk-'. $i .'">',
          );
          // UNIT
          $form['xls_review_fldset']['fldset_' . $i]['txt_unit_' . $i] = array(
            '#type' => 'textfield',
            '#title' => t('Unit'),
            '#required' => TRUE,
            '#description' => t('Unit of measurments used'),
            '#size' => 30,
            '#maxlength' => 100,
          );
          // DESCRIPTION - describe the trait.
          $form['xls_review_fldset']['fldset_' . $i]['txtarea_describe_' . $i] = array(
            '#type' => 'textarea',
            '#title' => t('Describe the method used'),
            '#required' => TRUE,
            '#description' => t('Describe the method used to collect this data if you used a scale, be specific'),
          );
        }
    }
  } 
  
   return $form;
}

/*
 * Selects the piece of the form we want to use as replacement text and returns
 * it as a form (renderable array).
 *
 * @return renderable array (the trait description elements)
 */
function ajax_rawpheno_upload_form_step2_expand_trait_callback($form, $form_state) {
  $trait_index = $form_state['triggering_element']['#ajax']['trait_index'];
  return $form['xls_review_fldset']['fldset_' . $trait_index];
}

/**
 * Function callback: Construct form for Stage 03.
 *
 * Stage 03 form is the final stage that displays a status message
 * and a navigation button to direct user after a successful file upload.
 */
function rawpheno_upload_form_stage_save($form, &$form_state) {

  $job_id = NULL;
  $percent_complete = 0;
  $message = 'Waiting for job system';
  if (isset($form_state['build_info']['args'][0])) {
    $job_id = $form_state['build_info']['args'][0];
    
    // Retrieve the tripal job and determine the percent complete.
    $job = tripal_get_job($job_id);

    if ($job) {

      // Set the progress bar based on the job.
      if ($job->status == 'Running') {
        $percent_complete = $job->progress;
        $message = 'Loading Spreadsheet...';
      }
      elseif($job->status == 'Completed') {
        $message = 'Complete!';
        $percent_complete = 100;
      }
      elseif($job->status == 'Waiting') {
        // Default no need to set.
      }
      else {
        $message = $job->status;
        $percent_complete = $job->progress;
        drupal_set_message('It appears something has gone wrong loading your spreadsheet. Please contact the administrator.', 'error');
      }

      if ($job->status == 'Running' OR $job->status == 'Waiting') {
        $form['#attached']['js'][] = array(
          'type' => 'inline',
          'data' => '
            setTimeout(function(){
              window.location.reload(1);
            }, 5000);'
        );
      }
    }
    else {
      drupal_set_message('We are unable to find the job tasked with loading your spreadsheet. Please contact the administrator.','error');
    }
  }

  $form['notice'] = array(
    '#type' => 'markup',
    '#markup' => '<div class="messages status">Your spreadsheet has been successfully submitted and <em>will not be interupted</em> if you choose to leave this page.</div>'
      . '<div class="messages warning">The progress bar below indicates our progress updating KnowPulse. <em>Your data will not be available until the progress bar below completes</em>.</div>'
      . '<br />'
  );
  
  // We make a DIV which the progress bar can occupy. You can see this in use
  // in ajax_example_progressbar_callback().
  $form['status'] = array(
    '#type' => 'markup',
    '#markup' => theme('progress_bar', array('percent' => $percent_complete, 'message' => $message)),
  );
  
  $form['buttons'] = array(
    '#type' => 'markup',
    '#prefix' => '<div class="upload-buttons step3">',
    '#suffix' => '</div>'
  );
  
  $form['buttons']['new_upload'] = array(
    '#type' => 'markup',
    '#prefix' => '<a href="' . url('phenotypes/raw/upload') . '" target="_blank">',
    '#markup' => '<div class="upload-icon-link"><img alt="Upload more Phenotypic Data" src="data:image/png;base64,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" />'
      . '<div class="upload-icon-title">Upload New Data</div></div>',
    '#suffix' => '</a>'
  );

  $form['buttons']['summary'] = array(
    '#type' => 'markup',
    '#prefix' => '<a href="' . url('phenotypes/raw') . '" target="_blank">',
    '#markup' => '<div class="upload-icon-link"><img alt="" src="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAIAAAACACAYAAADDPmHLAAAFlklEQVR4Xu2dO28TQRDHOQoeBQQhRUpLEUFtu6SgixLafAAQHU0kGpIqooI0SGnoEHyAtOTRUVDarkEpaKNEQgQKHgVhFoUIny/e2fE6t7f8LAUL339nd2b/N4+9xxbnjj+dTmf+6OhoSf7bkr/pv7/znZUFDkSbflEU691ud8tpVrh/2u32M/l6nJWqKOOzwFqv11sujs/8TR+a4/lZQDzBQiFn/7aoNpefemiksMCOI8A+MV9hqjwhB44AR3nqhlYaC0AAjZUyxkCAjCdXoxoE0FgpY0zqBNgT2x9GtP+syDrvkfdLju9G7HNKZM0o5LlFmk8KnBZyQ4AXfOCkCSArk/f7/f5rnxLa45Lwfhasm5BRn0NZILmmlenDtVqte1Jvv/Lh5PiK9OsW5KJ8RNf3IuimTxgEGLYQBPCx5qyO4wHslsYDVNiOEDBsFEIAISDdlUBCACGAKsDIAXIAcgDKwDIHSAJJAlkIKnGAKoAqgCqgxAFWAo2JZ/RmlIF2k1IFUAVQBVAFcDVwgAOUgZSBlIGUgdwQ8i8HWAdgHSDddQCZmw3569mLoaGWq/LLJY+873L8ScQ+2yJrUSFvRzBvFTgt5JEAvQ/5Ju0BtJqCs1sAAthtl0VLCJDFNNqVgAB222XREgJkMY12JSCA3XZZtIQAWUyjXQkIYLddFi0hQBbTaFdCS4A9uTljxd7NYEt5WPKucnUsVpchcr6Jrg9DGozCiq635fgDhbwN6feNAqeCSL9PBeh9KllLgA/y5OotVc8KkFyWXRaYG2CKH24Jq5gVCGCkai6Ph0MACEAIsHAAD1BhNXKASiol/YYQQoDl9Jc2eAA8QBbvCMID4AFIAi0cIAQQAggBZQ5QBVAFsBQ8zAHKQEuMnUAbrgVwLYBXxZY5QBlodDVUAVQBVAFUAc1/WzghgBDASqCFA7nkAG4Xja8WA5zS5qL87ntKN2J3QaLcLmpfglqMBrtdOy4r5Lmnkn8ocFrIFQH6dkc5p70nUNspuIZZAAI0bMJiDxcCxLZow+RBgIZNWOzhQoDYFm2YPAjQsAmLPVwIENuiDZMHARo2YbGHqyVAXQtB30ThnxGVviqyiojyYopKeiGolmsBNb0uPuakhsjijqCytSBACH8GsbH3C8AD2OdC2xIPgAdId/dwPID2PLbj8AB4ADzAAAdIAu3uhCSwwnbKLWPsVh+vJSGAEEAIIASM50VOWhMCCAFR9w2kDIx0Zo4QQw5ADkAOQA4QydOQA5ADkAOUOcA6wPBZob0hhCQwkmsmCRQLaN8RxFKwnXXkAOQAUXOAujaMeCle4J39PBhsKZsovJBfNA9qxuoyRE7SG0aEKAK2QRbQJoENUomhhlgAAoRYK0MsBMhwUkNUggAh1soQCwEynNQQlSBAiLUyxEKADCc1RCUIEGKtDLFaAhyI7s8j6n9HZM0p5G0IpqfAaSGrAvS9ns49pftEK1CBawtmUYGrBaIlwP90NbCu18VDgLIFaroaCAEqqIgHMJ6fAa+KNfYwXjNCwLD98AB4gFp2DBnvVDa2xgPgAdru7di+DzmAz0KnHCcHqDBM4vcEkgOQA5ADlDlACCAEnP2WMSwEGVkX0IwqgCqAKqDEAZJAkkCSQJLAgDg6Cso6AOsA2q1jI1EuTAxJIEkgSSBJoN9rsBDkt1ElIpccwO3a8dFog6pm1+XHaYW8PcEcKnBayKwAfduput1RdrUCFbgpwcwocLVAtDlALYOj08lbAAJM3sZJ9wABkp6eyQ8OAkzexkn34Aiwr0zIklaEwZkscOAIsC1NNU/pmHqgUdIW2Ck6nc68XHffTHqYDG4iFpCXZi382UVTvMAz+Xo8kV4QmqoF1nq93vLJNqrHnmBJRtsiJ0h1zsYel3vIty9n/nq3291y0n4DHetPXYnj9WcAAAAASUVORK5CYII=" />'
      . '<div class="upload-icon-title">Data Summary</div></div>',
    '#suffix' => '</a>'
  );
  
  $form['buttons']['download'] = array(
    '#type' => 'markup',
    '#prefix' => '<a href="' . url('phenotypes/raw/download') . '" target="_blank">',
    '#markup' => '<div class="upload-icon-link"><img alt="" src="data:image/png;base64,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" />'
      . '<div class="upload-icon-title">Download Data</div></div>',
    '#suffix' => '</a>'
  );
  
  return $form;
}

/**
 * Implements hook_file_validate().
 *
 * Basic compliance test of spreadsheet submitted.
 */
function rawpheno_file_validate($file) {
  // Variables containing spreadsheet file information.
  $xls_file = drupal_realpath($file->uri);
  $xls_extension = pathinfo($file->filename, PATHINFO_EXTENSION);  

  // Assume everything went well unless it didn't.
  $status = array(
    'is_xlsx' => TRUE,
    'tab_exists' => TRUE,
    'missing_header' => TRUE,
  );
  
  if (isset($file) && filesize($xls_file) && in_array($xls_extension, array('xls', 'xlsx'))) {
    // File exists and is valid xls or xlsx.
    // Array to hold first, last and 10th row required in performing basic compliance test.
    $arr_xls_rows = rawpheno_function_read($file);
    // Total number of rows returned.
    $total_rows = count($arr_xls_rows);
    
    // @todo improve this check.
    if (!$arr_xls_rows) {
      $status['is_xlsx'] = FALSE;
    }

    if ($total_rows < 2) {
      // Missing measurement sheet or is empty worksheet.
      $h = 'No data to process or Measurements sheet is missing';
      $txt_error = rawpheno_function_format($h, 'em');
      $txt_error .= 'Please check spreadsheet for missing or blank sheet and try again.';
      $status['tab_exists'] = FALSE;
    }
    else {
      // VALIDATE HEADER & ROWS
      // Array to hold errors found in the spreadsheet.
      $arr_xls_errors = array();
      // Get the column headers row.
      $xls_headers_row = $arr_xls_rows[0];

      // Validate column headers row.
      // Index 0 - Headers row.
      $arr_xls_errors[0] = rawpheno_function_chkheader($xls_headers_row);
      $status['missing_header'] = $arr_xls_errors[0];
      
      // Validate non column header rows.
      /* @todo disabled until I can re-do validation to use hooks.
      // Index 1 - First row.
      $arr_xls_errors[1] = rawpheno_function_chkrow($arr_xls_rows[1], $xls_headers_row);
      // Last or 10th row.
      if ($total_rows > 2) {
        // Get the index of last or 10th row.
        $i = ($total_rows < 11) ? $total_rows - 1 : 10;
        $arr_xls_errors[2] = rawpheno_function_chkrow($arr_xls_rows[$i], $xls_headers_row);
      }
*/
      
      // Format all errors and report a list of errors found in the spreadsheet.
      $txt_error = rawpheno_function_error($arr_xls_errors, $total_rows);
    }  

    drupal_set_message(theme('rawpheno_upload_validation_report', array('status' => $status)), 'rawpheno-validate-progress');

    //Determine if there is errors to report.    
    if (strlen(trim($txt_error)) > 0) {
      // Errors found.
      $sheet_error = rawpheno_function_format('In Worksheet Measurements:', 'em');
      return FALSE;//'<pre id="stage01-window-error">'.$sheet_error.$txt_error.'</pre>';
    } 
    else {
      // No errors - save new header and path to file.
      variable_set('rawpheno_xls_file', drupal_realpath($file->destination));
      
      drupal_set_message(t('<span id="rawpheno-upload-successful">The specified file %file was uploaded successfully after passing all validation.</span>', array('%file' => $file->filename)));
    }
  }  

}

/**
 * Implements hook_validate().
 */
function rawpheno_upload_form_master_validate($form, &$form_state) { }

/**
 * Implements hook_submit().
 *
 * Master submit to handle form submit.
 */
function rawpheno_upload_form_master_submit(&$form, &$form_state) {

  // Which button triggers a submit action.
  $btn_submit = $form_state['triggering_element']['#value'];  

  // Save any additional traits and then submit a job to save the spreadsheet.
  if ($form_state['stage'] == 'review') {
    $job_id = rawpheno_submit_review($form, $form_state);

    // Then we need to add the job_id to the path so the system can keep track of it.
    if ($job_id) {
      drupal_goto(current_path() . '/' . $job_id);
    }
  }
  
  // If we just uploaded the file then we want to save the fid for easy access.
  if (isset($form_state['values']['dnd'])) {
    $form_state['multistep_values']['fid'] = $form_state['values']['dnd'];
  }
    
  // If the next step button was pressed then iterate to the next step.
  if ($btn_submit == 'Next Step') {
    // Definitely save the form id.
    if(isset($form_state['multistep_values']['form_build_id'])) {
      $form_state['values']['form_build_id'] = $form_state['multistep_values']['form_build_id'];
    }
    
    // Save the values from the current step.
    $form_state['multistep_values'][$form_state['stage']] = $form_state['values'];

    // Iterate to the next step.
    $form_state['new_stage'] = rawpheno_next_page($form, $form_state);

    // Ensure the form state is saved and the form is rebuilt.  
    $form_state['multistep_values']['form_build_id'] = $form_state['values']['form_build_id'];
    $form_state['stage'] = $form_state['new_stage'];
    $form_state['rebuild'] = TRUE;
  }
}
  
/**
 * Save spreadsheet to database.
 */
function rawpheno_submit_review($form, &$form_state) {
  // Save spreadsheet data in the following order.
  // 1. New column headers.
  // 2. The entire spreadsheet.
  
  // cvterm id of controlled vocabulary. 
  $cvid = tripal_get_cv(array('name' => 'phenotype_measurement_units'));
  $cv_measurements_unit = $cvid->cv_id;
 
  // 1. Save new headers.
  // Read variable that holds new column headers.
  $new_header = $form_state['multistep_values']['new_headers'];
  
  // Determine if there is new header.
  if (count($new_header) > 0) {
  
    // Read each column header.
    foreach($new_header as $i => $header) { 
    
      // Determine if the form in review traits has been filled out and checkbox
      // has been checked by user. If it has been chcked then save the trait.
      if ($form_state['values']['chk_' . $i] == 1) {  
      
        // First, save the measurement.
        $m_cvterm = array(
          'id' => 'tripal:' . $form_state['values']['txt_header_' . $i],
          'name' => $form_state['values']['txt_header_' . $i],
          'definition' => $form_state['values']['txt_def_' . $i],
          'cv_name' => 'phenotype_measurement_types'
        );
        $m_cvterm_id = chado_select_record('cvterm',array('cvterm_id'), array('name' => $m_cvterm['name'], 'cv_id' => array('name' => $m_cvterm['cv_name'])));
        if (!$m_cvterm_id) {
          $m_cvterm_id = tripal_insert_cvterm($m_cvterm);
        }
        // Grab just the id.
        if (is_array($m_cvterm_id)) {
          $m_cvterm_id = $m_cvterm_id[0]->cvterm_id;
        }
        elseif (is_object($m_cvterm_id)) {
          $m_cvterm_id = $m_cvterm_id->cvterm_id;
        }
        
        // Then save the Unit.
        $u_cvterm = array(
          'id' => 'tripal:' . $form_state['values']['txt_unit_' . $i],
          'name' => $form_state['values']['txt_unit_' . $i],
          'definition' => $form_state['values']['txt_unit_' . $i],
          'cv_name' => 'phenotype_measurement_units'
        );
        $u_cvterm_id = chado_select_record('cvterm',array('cvterm_id'), array('name' => $u_cvterm['name'], 'cv_id' => array('name' => $u_cvterm['cv_name'])));
        if (!$u_cvterm_id) {
          $u_cvterm_id = tripal_insert_cvterm($u_cvterm);
        }
        // Grab just the id.
        if (is_array($u_cvterm_id)) {
          $u_cvterm_id = $u_cvterm_id[0]->cvterm_id;
        }
        elseif (is_object($u_cvterm_id)) {
          $u_cvterm_id = $u_cvterm_id->cvterm_id;
        }
        
        // Don't forget the method description.
        // @todo this shouldn't be using this type.
        // @todo should this even be attached to the unit? I had assumed the trait.
        $prop = array(
          'cvterm_id' => $u_cvterm_id,
          'type_id' => $cv_measurements_unit,
          'value' => $form_state['values']['txtarea_describe_' . $i],
          'rank' => 0,
        );
        $prop_id = chado_select_record('cvtermprop', array('cvtermprop_id'), $prop);
        if (!$prop_id) {
          $prop = chado_insert_record('cvtermprop', $prop);
        }
        
        // Finally relate the measurement and unit.
        // @todo: this shouldn't be using this type.
        $rel = array(
          'subject_id' => $u_cvterm_id,
          'type_id' => $cv_measurements_unit,
          'object_id' => $m_cvterm_id,
        );
        $rel_id = chado_select_record('cvterm_relationship', array('cvterm_relationship_id'), $rel);
        if (!$rel_id) {
          chado_insert_record('cvterm_relationship', $rel);
        }
      }
    } 
  }
  
  // 2. The entire spreadsheet.
  // Get the variable that holds the path to the spreadsheet file in the server.
  $xls_file = variable_get('rawpheno_xls_file'); 
  
  // Array of required traits excluding Name.
  $plantprop_headers = rawpheno_function_headers('plantprop');

  global $user;
  if (isset($xls_file) && !empty($xls_file)) {
    $job_id = tripal_add_job(
      "Upload Phenoypic data: ".$xls_file,
      'rawpheno',
      'rawpheno_load_spreadsheet',
      array(
        $xls_file,
        serialize($plantprop_headers)
      ),
      $user->uid
    );

    return $job_id;
  }
  
  // After saving process, reset all variables.
  if ($form_state['triggering_element']['#value'] == 'Save spreadheet') {
    rawpheno_function_resetvar();
  }
}
