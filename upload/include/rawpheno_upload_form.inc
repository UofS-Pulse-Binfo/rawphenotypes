<?php

/**
 * @file
 * Contains all implementations of hook_form().
 */

//Include other implementations, hook_submit() and hook_validate().
module_load_include('inc', 'rawpheno', 'upload/include/rawpheno_upload_navigation');
module_load_include('inc', 'rawpheno', 'upload/include/rawpheno_upload_submit');
module_load_include('inc', 'rawpheno', 'upload/include/rawpheno_upload_validate');
//Include functions required in processing spreadsheet file.
module_load_include('inc', 'rawpheno', 'upload/include/rawpheno_upload_function');

/**
 * Master form which calls an individual form for each step.
 * 
 * @param type $form
 * @param type $form_state
 * @return type
 */
function rawpheno_element($form, &$form_state) {
  //On the first load of the page.
  if (!isset($form_state['stage'])) {
    //Reset variables before starting.
    rawpheno_function_resetvar();
    //Default to stage 01 - upload and validate spreadsheet.
    $form_state['stage'] = 'check';
  }
  
  $form = array();
  
  //Create a page title based on the current stage.
  $form = rawpheno_get_header($form, $form_state);
  
  //Load corresponding interface or form.
  switch($form_state['stage']) {
    case 'check':
      //Stage 01 - form drag and drop uploader.
      return rawpheno_form_check($form, $form_state);
      break;

    case 'review':
      //Stage 02 - describe form, optional, only when there is additional trait.
      return rawpheno_form_review($form, $form_state);
      break;
	 
    case 'save':
      //Stage 03 - success page.
      return rawpheno_form_save($form, $form_state);
      break;
  }

  return $form;
}

/**
 * Form stage 01 - upload spreadsheet and perform
 * a basic compliance test.
 * 
 * @param type $form
 * @param type $form_state
 * @return type
 */
function rawpheno_form_check($form, &$form_state) {
  //Create an instance of DragNDrop Upload with the following settings:
  //SETTINGS:
  //file_upload_max_size: max file size allowed
  //upload location: destination of file
  //upload event: manual - show an upload button or auto - uploads after drag drop
  $form['dnd'] = array(
    '#type' => 'dragndrop_upload',
    '#file_upload_max_size' => '10M',
    '#upload_location' => 'public://',
    '#upload_event' => 'manual',
  );
  
  //Save spreadsheet button - go to stage 03.
  $form['stage01_submit_save'] = array(
    '#type' => 'submit',
    '#value' => 'Save spreadheet',
  );
  
  //Describe trait button - go to stage 02.
  $form['stage01_submit_review'] = array(
    '#type' => 'submit',
    '#value' => 'Describe additional traits/column headers',
  );
  
  //Close error window button
  $form['stage01_button_close'] = array(
    '#markup' => '<br /><a class="button" id="edit-stage01-button-close" href="javascript:void();">
                  Close error and try again</a>',
  );
  
  //Attach CSS and JavaScript.
  $form['#attached']['css'] = array(drupal_get_path('module', 'rawpheno') . '/upload/css/rawpheno_upload_stage01.css');
  $form['#attached']['js'] = array(drupal_get_path('module', 'rawpheno') . '/upload/js/rawpheno_upload_stage01.js');

  return $form;  
}

/**
 * Form stage 02 - review new traits found.
 * Allow user to describe and save a additional trait/s found
 * in the spreadsheet.
 *
 * @param type $form
 * @param type $form_state
 * @return form
 */
function rawpheno_form_review($form, &$form_state) {
  //This is the error message used in lieu of Drupal form_set_error().
  //This error message will be in close proximity to the interface.
  $h = 'Error in checked column headers/traits. Please review form for any empty fields';
  $h = rawpheno_function_format($h, 'em');
  $form['stage02_window_error'] = array(
    '#markup' => '<div id="stage02-window-error" class="messages error">Required fields empty.<pre>'.$h.
                 '</pre></div>',
  );
  //
  
  //Main fieldset container for form elements 
  $form['xls_review_fldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Check the traits that you want to describe and save'),
  );
  
  //Read variable containing new headers found in stage 01.
  //Based on these new traits, construct form required to describe traits.
  $new_header = variable_get('rawpheno_new_header');
  if (isset($new_header) && count($new_header) > 0) {
    ///Build form only when new trait variable has value.
    foreach($new_header as $i => $k) {
      if (isset($k) && !empty($k)) {
        //Fieldset to contain a set of trait with corresponding form elements.
        $form['xls_review_fldset']['fldset_' . $i] = array(
          '#type' => 'fieldset',
          '#title' => '',
        );
          //All fields are required.
          //CHECKBOX to let user select a trait to describe and save.
          $form['xls_review_fldset']['fldset_' . $i]['chk_' . $i] = array(
            '#type' => 'checkbox',
            '#title' => t(ucwords($k)),
          );
          //TERM NAME/TRAIT/HEADER
          $form['xls_review_fldset']['fldset_' . $i]['txt_header_' . $i] = array(
            '#type' => 'hidden',
            '#value' => $k,
          ); 
            //These fields are hidden by default - show only when check.
            //TERM DEFINITION
            $form['xls_review_fldset']['fldset_' . $i]['txt_def_' . $i] = array(
              '#type' => 'textfield',
              '#title' => t('Definition*'),
              '#description' => t('A human-readable text definition'),
              '#maxlength' => 150,
              '#prefix' => '<div id="div-edit-chk-'. $i .'" style="display: none;">',
            );
            //UNIT
            $form['xls_review_fldset']['fldset_' . $i]['txt_unit_' . $i] = array(
              '#type' => 'textfield',
              '#title' => t('Unit*'),
              '#description' => t('Unit of measurments used'),
              '#size' => 30,
              '#maxlength' => 100,
            );
            //DESCRIPTION - describe the trait including the unit used.
            $form['xls_review_fldset']['fldset_' . $i]['txtarea_describe_' . $i] = array(
              '#type' => 'textarea',
              '#title' => t('Describe the method used*'),
              '#description' => t('Describe the method used to collect this data if you used a scale, be specific'),
            );
            //Note indicating fields must have a value.
            $form['xls_review_fldset']['fldset_' . $i]['required_mark'] = array(
              '#markup' => '<small>* means required</small>',
              '#suffix' => '</div>',
            );
      }
    } 
    ///
  
    //Save spreadsheet button - go to stage 03
    $form['stage02_submit_save'] = array(
      '#type' => 'submit',
      '#value' => 'Save spreadheet',
    );
  } 
  else {
    //Prevent any processes in case user will navigate
    //using the browser's back and forward button. 
    //Lead user to stage 01.
    $form['stage02_submit_save'] = array(
      '#type' => 'submit',
      '#value' => 'Upload another spreadsheet',
    );
  }
  
  //Attach CSS and JavaScript.
  $form['#attached']['css'] = array(drupal_get_path('module', 'rawpheno') . '/upload/css/rawpheno_upload_stage02.css');
  $form['#attached']['js'] = array(drupal_get_path('module', 'rawpheno') . '/upload/js/rawpheno_upload_stage02.js');
  
  return $form;
}

/**
 * Form stage 03 - final stage - save the spreadsheet to database.
 *
 * @param type $form
 * @param type $form_state
 * @return type
 */
function rawpheno_form_save($form, &$form_state) {
  //Success message to display.
  $h = 'Thank you! your file has been submitted';
  $h = rawpheno_function_format($h, 'em');
  $form['stage03_window_success'] = array(
    '#markup' => '<div id="stage03-window-success" class="messages status">Spreadsheet submitted<pre>'.$h.
                 '</pre></div><br />',
  );
  //
  
  //Buttons to lead user to other parts of the site.
  //Upload another spreadsheet button - go to stage 01.
  $form['stage03_submit_save'] = array(
    '#type' => 'submit',
    '#value' => 'Upload another spreadsheet',
  );
  
  //Home buttons - go to home KnowPulse.
  $form['stage03_submit_home'] = array(
    '#type' => 'submit',
    '#value' => 'Go to home KnowPulse',
  );
  
  //Attach CSS.
  $form['#attached']['css'] = array(drupal_get_path('module', 'rawpheno') . '/upload/css/rawpheno_upload_stage03.css');

  return $form;
}