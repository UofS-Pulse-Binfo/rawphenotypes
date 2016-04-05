<?php

/**
 * @file
 * Contains the upload raw phenotypic data form.
 */

// Additional Helper functions
module_load_include('inc', 'rawpheno', 'include/rawpheno.upload.helpers');
// Include functions required in processing spreadsheet file.
module_load_include('inc', 'rawpheno', 'include/rawpheno.upload.excel');

/**
 * Master form constructor which calls an individual form for each stage.
 * 
 * @see rawpheno_element_validate()
 * @see rawpheno_element_submit()
 */
function rawpheno_upload_form_master($form, &$form_state) {    
  // Navigation button. Related page of upload page is instructions page.
  $form['page_button'] = array(
    '#type' => 'markup',
    '#markup' => t('Standard Procedure &#10095;'),
  );
  
  // If the stage is not set then default to the first stage (i.e. 'check' )
  if (!isset($form_state['stage'])) {
    $form_state['stage'] = 'check';
  } 
  
  // If a job_id was provide in the URL then the user wants information
  // on a prvious bulk loading job. Thus we should show them the last step.
  if (isset($form_state['build_info']['args'][0])) {
    $form_state['stage'] = 'save';
  }
  
  // Add the stage tracker/header.
  $form = rawpheno_get_header($form, $form_state);

  // Stage indicator for theme function.
  $form['current_stage'] = array(
    '#type' => 'value',
    '#value' => $form_state['stage'],
  );

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
      // Attach JS Collapse library.
      $form['#attached']['library'][] = array('system', 'drupal.collapse');

      // Stage 01 - upload and check spreadsheet.
      $form['#attached']['css'] = array($path . 'css/rawpheno.upload.stage01.css');
      $form['#attached']['js']  = array($path . 'js/rawpheno.upload.stage01.js',
                                        $path . 'js/rawpheno.upload.script.js');
      
      $form = rawpheno_upload_form_stage_check($form, $form_state);
      break;

    case 'review':
      // Stage 02 - describe form (only when there is additional trait).
      $form['#attached']['css'] = array($path . 'css/rawpheno.upload.stage02.css');
      $form['#attached']['js']  = array($path . 'js/rawpheno.upload.script.js');

      $form = rawpheno_upload_form_stage_review($form, $form_state);
      break;
	 
    case 'save':
      // Stage 03 - save to databse and success page.
      $form['#attached']['css'] = array($path . 'css/rawpheno.upload.stage03.css');
      $form['#attached']['js']  = array($path . 'js/rawpheno.upload.script.js');

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
  //  #file_upload_max_size: max file size allowed
  //  #upload_location: destination of file
  //  #upload_event: manual - show an upload button or auto - uploads after drag drop
  //  #upload_validators: allowed file extensions
  //  #upload_button_text: label of upload button
  //  #droppable_area_text: text in drop area
  //  #progress_indicator: none, throbber or bar
  //  #progress_message: message to display while processing
  //  #allow_replace: allow user to replace file by drag and drop another file
  //  #standard_upload: show browse button or not
  $form['dnd'] = array(
    '#type' => 'dragndrop_upload',
    '#file_upload_max_size' => '10M',
    '#upload_location' => 'public://',
    '#upload_event' => 'auto',
    '#upload_validators' => array('file_validate_extensions' => array('xlsx xls')),
    '#droppable_area_text' => t('Drag Phenotype Data Collection file here'),
    '#progress_indicator' => 'throbber',
    '#progress_message' => 'Please wait...',
    '#allow_replace' => 1,
    '#standard_upload' => 1,
    // We are adding our own element process function so that we can make a successfully
    // uploaded/validated file permanent during the AJAX process rather than waiting for
    // them to click the "Next" button.
    '#process' => array(
      'file_managed_file_process',
      'dragndrop_upload_element_element_process',
      'rawpheno_phenotype_upload_file_element_process',
    ),
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
  
  //Remove this after applying theme to describe trait.
  foreach($new_header as $i => $k) {
    if (isset($k) && !empty($k)) {
    
      // All fields are required.
      // CHECKBOX to let user select a trait to describe and save.
      $form['xls_review_fldset']['chk_' . $i] = array(
        '#type' => 'checkbox',
        '#title' => t(ucwords($k)),
        '#ajax' => array(
          'callback' => 'ajax_rawpheno_upload_form_step2_expand_trait_callback',
          'wrapper' => 'trait-description-' . $i,
          'effect' => 'fade',
          'trait_index' => $i,
        )
      );
  
      // Container div that holds form elements.
      $form['xls_review_fldset']['fldset_' . $i] = array(
        '#type' => 'markup',
        '#prefix' => '<div id="trait-description-' . $i . '" class="container-trait">',
        '#suffix' => '</div>',
      );
        
      // If the checkbox is checked then show the fields user want to described.
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
            '#maxlength' => 200,
          );
          // UNIT
          $form['xls_review_fldset']['fldset_' . $i]['txt_unit_' . $i] = array(
            '#type' => 'textfield',
            '#title' => t('Unit'),
            '#required' => TRUE,
            '#description' => t('Unit of measurments used'),
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
  
  return $form;
}

/**
 * Implements hook_file_validate().
 *
 * Basic compliance test of spreadsheet submitted.
 */
function rawpheno_file_validate($file) {

  if ($file->source == 'dnd') {
    // Validate the file.
    // The following function will return an array specifying which of the validation
    // steps passed and providing infomration for those that failed.
    $status = rawpheno_validate_excel_file($file);
  
    // We want to show the user which steps passed/failed even if all of them passed,
    // so lets do that now. We use drupal_set_message() because returning from this function
    // creates an error message and halts file upload, whereas, using drupal_set_message()
    // allows us to print to the screen regardless of failure/success.
    drupal_set_message(theme('rawpheno_upload_validation_report', array('status' => $status)), 'rawpheno-validate-progress');
  
    // Now we want to determine if validation passed or failed as a whole.
    // To do that we have to look at each step and only if all steps passed 
    // did the file pass validation and can be uploaded.
    $all_passed = TRUE;
    foreach ($status as $test_result) {
      if ($test_result !== TRUE) {
        $all_passed = FALSE;
        break; break;
      }
    }
  
    // hook_file_validate() expects an array of error messages if validation failed and
    // and empty array if there are no errors. We don't want this system to print the errors
    // for us since we are using our more friendly theme (see drupal_set_message() above).
    // The work-around is to pass FALSE if validation failed.
    if ($all_passed) {
      drupal_set_message('<span id="rawpheno-upload-successful">Your file uploaded successfully. Please click "Next" to continue.</span>');
      return array();
    }
    else {
      return FALSE;
    }
  }
}

/**
 * Make the phenotype excel file permanent on successful upload.
 *
 * This is an additional process handler used by the form API to generate the form array
 * for a given element. Usually it is used to make a custom form element or enhance a 
 * standard form element. 
 *
 * We are using it to capture the just uploaded file within the AJAX call by checking
 * when the element is rendered if it has a fid (ie: has been saved).
 */
function rawpheno_phenotype_upload_file_element_process($element, &$form_state, $form) {
  
  if (isset($element['#value']['fid']) AND !empty($element['#value']['fid'])) {

    $file = file_load($element['#value']['fid']);
    if ($file) {
    
      // Make the file permanent.
      $file->status = FILE_STATUS_PERMANENT;
      file_save($file);
    
      // Also, point out that we are using it ;-)
      // Note, the file_usage_add() function expects a numerical unique id which we don't have.
      // We have gotten around this by using the uid concatenated with the timestamp using
      // the assumption that a single user cannot upload more than one phenotype file within a second.
      file_usage_add($file, 'rawpheno', 'rawphenotypes-file', $file->uid . $file->timestamp);  
    }
  
  }
  
  return $element;
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
  $file = file_load($form_state['multistep_values']['fid']);
  $xls_file = drupal_realpath($file->uri);
  
  // Array of required traits excluding Name.
  $plantprop_headers = rawpheno_function_headers('plantprop');

  global $user;
  if (isset($xls_file) && !empty($xls_file)) {
    $job_id = tripal_add_job(
      "Upload Phenoypic data: ".$xls_file,
      'rawpheno',
      'rawpheno_load_spreadsheet',
      array(
        $form_state['multistep_values']['fid'],
        serialize($plantprop_headers)
      ),
      $user->uid
    );

    return $job_id;
  }
}
