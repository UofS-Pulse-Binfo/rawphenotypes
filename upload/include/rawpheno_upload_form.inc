<?php

/**
 * @file
 * Contains all implementations of hook_form().
 */

// Include other implementations of hook_submit() and hook_validate().
module_load_include('inc', 'rawpheno', 'upload/include/rawpheno_upload_navigation');
module_load_include('inc', 'rawpheno', 'upload/include/rawpheno_upload_submit');
module_load_include('inc', 'rawpheno', 'upload/include/rawpheno_upload_validate');
// Include functions required in processing spreadsheet file.
module_load_include('inc', 'rawpheno', 'upload/include/rawpheno_upload_function');

/**
 * Master form constructor which calls an individual form for each stage.
 * 
 * @see rawpheno_element_validate()
 * @see rawpheno_element_submit()
 */
function rawpheno_upload_form_master($form, &$form_state) {  
  // Reset variables before starting.
  if (!isset($form_state['stage'])) {
    rawpheno_function_resetvar();
    // Default to stage 01.
    $form_state['stage'] = 'check';
  }
  
  // Create a page title based on the current stage.
  // This includes the current stage tracker.
  $form = rawpheno_get_header($form, $form_state);
  
  // Get the directory path to rawpheno module.
  $path = drupal_get_path('module', 'rawpheno') . '/upload/';
  
  // Load corresponding form together with JavaScript and CSS.
  switch($form_state['stage']) {
    case 'check':
      // Stage 01 - upload and check spreadsheet.
      $form['#attached']['css'] = array($path . 'css/rawpheno_upload_stage01.css');
      $form['#attached']['js']  = array($path . 'js/rawpheno_upload_stage01.js');
      
      $form = rawpheno_form_check($form, $form_state);
      break;

    case 'review':
      // Stage 02 - describe form (only when there is additional trait).
      $form['#attached']['css'] = array($path . 'css/rawpheno_upload_stage02.css');
      $form['#attached']['js']  = array($path . 'js/rawpheno_upload_stage02.js');
      
      $form = rawpheno_form_review($form, $form_state);
      break;
	 
    case 'save':
      // Stage 03 - save to databse and success page.
      $form['#attached']['css'] = array($path . 'css/rawpheno_upload_stage03.css');

      $form = rawpheno_form_save($form, $form_state);
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
function rawpheno_form_check($form, &$form_state) {
  // Create an instance of DragNDrop Upload.
  // SETTINGS:
  //  file_upload_max_size: max file size allowed
  //  upload location: destination of file
  //  upload event: manual - show an upload button or auto - uploads after drag drop
  $form['dnd'] = array(
    '#type' => 'dragndrop_upload',
    '#file_upload_max_size' => '10M',
    '#upload_location' => 'public://',
    '#upload_event' => 'manual',
  );
  
  // Save spreadsheet button - go to stage 03.
  $form['stage01_submit_save'] = array(
    '#type' => 'submit',
    '#value' => 'Save spreadheet',
  );
  
  // Describe trait button - go to stage 02.
  $form['stage01_submit_review'] = array(
    '#type' => 'submit',
    '#value' => 'Describe additional traits/column headers',
  );
  
  // Close error window button
  $form['stage01_button_close'] = array(
    '#markup' => '<br /><input type="button" class="form-submit" id="edit-stage01-button-close" value="Close error and try again">',
  );

  return $form;  
}

/**
 * Function callback: Construct form for Stage 02.
 *
 * Stage 02 form allows user to describe and save a additional trait/s found
 * in the spreadsheet submitted in Stage 01.
 */
function rawpheno_form_review($form, &$form_state) {
  // This is the error message used in lieu of Drupal form_set_error().
  // This error message will be in close proximity to the interface.
  $h = 'Error in checked column headers/traits. Please review form for any empty fields';
  $h = rawpheno_function_format($h, 'em');
  $form['stage02_window_error'] = array(
    '#markup' => '<div id="stage02-window-error" class="messages error">Required fields empty.<pre>'.$h.
                 '</pre></div>',
  );
  
  // Main fieldset container for form elements 
  $form['xls_review_fldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Check the traits that you want to describe and save'),
  );
  
  // Read variable containing new headers found in stage 01.
  // Based on these new traits, construct form to allow user to describe each new traits.
  $new_header = variable_get('rawpheno_new_header');
  
  // Create form only when there is new traits.
  if (isset($new_header) && count($new_header) > 0) {
    foreach($new_header as $i => $k) {
      if (isset($k) && !empty($k)) {
        // Fieldset to contain a set of trait and corresponding form elements.
        $form['xls_review_fldset']['fldset_' . $i] = array(
          '#type' => 'fieldset',
          '#title' => '',
        );
          // All fields are required.
          // CHECKBOX to let user select a trait to describe and save.
          $form['xls_review_fldset']['fldset_' . $i]['chk_' . $i] = array(
            '#type' => 'checkbox',
            '#title' => t(ucwords($k)),
          );
          // TERM NAME/TRAIT/HEADER
          $form['xls_review_fldset']['fldset_' . $i]['txt_header_' . $i] = array(
            '#type' => 'hidden',
            '#value' => $k,
          ); 
            // These fields are hidden by default - show only when checked.
            // TERM DEFINITION
            $form['xls_review_fldset']['fldset_' . $i]['txt_def_' . $i] = array(
              '#type' => 'textfield',
              '#title' => t('Definition*'),
              '#description' => t('A human-readable text definition'),
              '#maxlength' => 150,
              '#prefix' => '<div id="div-edit-chk-'. $i .'" style="display: none;">',
            );
            // UNIT
            $form['xls_review_fldset']['fldset_' . $i]['txt_unit_' . $i] = array(
              '#type' => 'textfield',
              '#title' => t('Unit*'),
              '#description' => t('Unit of measurments used'),
              '#size' => 30,
              '#maxlength' => 100,
            );
            // DESCRIPTION - describe the trait.
            $form['xls_review_fldset']['fldset_' . $i]['txtarea_describe_' . $i] = array(
              '#type' => 'textarea',
              '#title' => t('Describe the method used*'),
              '#description' => t('Describe the method used to collect this data if you used a scale, be specific'),
            );
            // Note indicating fields are required.
            $form['xls_review_fldset']['fldset_' . $i]['required_mark'] = array(
              '#markup' => '<small>* means required</small>',
              '#suffix' => '</div>',
            );
      }
    } 
  
    // Save spreadsheet button - go to stage 03
    $form['stage02_submit_save'] = array(
      '#type' => 'submit',
      '#value' => 'Save spreadheet',
    );
  } 
  else {
    // Direct user to stage 01 in case user will navigate
    // using back and forward button of the browser. 
    $form['stage02_submit_save'] = array(
      '#type' => 'submit',
      '#value' => 'Upload another spreadsheet',
    );
  }
  
   return $form;
}

/**
 * Function callback: Construct form for Stage 03.
 *
 * Stage 03 form is the final stage that displays a status message
 * and a navigation button to direct user after a successful file upload.
 */
function rawpheno_form_save($form, &$form_state) {
  // Success message to display.
  $h = 'Thank you! your file has been submitted';
  $h = rawpheno_function_format($h, 'em');
  $form['stage03_window_success'] = array(
    '#markup' => '<div id="stage03-window-success" class="messages status">Spreadsheet submitted<pre>'.$h.
                 '</pre></div><br />',
  );
  
  // Buttons to lead user to other parts of the site.
  // Upload another spreadsheet button - go to stage 01.
  $form['stage03_submit_save'] = array(
    '#type' => 'submit',
    '#value' => 'Upload another spreadsheet',
  );
  
  // Home buttons - go to home KnowPulse.
  $form['stage03_submit_home'] = array(
    '#type' => 'submit',
    '#value' => 'Go to home KnowPulse',
  );
  
  return $form;
}