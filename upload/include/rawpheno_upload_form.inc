<?php

/**
 * @file
 * implements hook form in phenotypes/upload page
 */
 
module_load_include('inc', 'rawpheno', 'upload/include/rawpheno_upload_navigation');
module_load_include('inc', 'rawpheno', 'upload/include/rawpheno_upload_submit');
module_load_include('inc', 'rawpheno', 'upload/include/rawpheno_upload_validate');
//handle parsing of spreadsheet file
module_load_include('inc', 'rawpheno', 'upload/include/rawpheno_upload_function');

/**
 * master form which calls an individual form for each step.
 * 
 * @param type $form
 * @param string $form_state
 * @return type
 */
function rawpheno_element($form, &$form_state) {
  //default to stage 01
  if (!isset($form_state['stage'])) {
    //reset variable before starting upload
    rawpheno_function_resetvar();
    $form_state['stage'] = 'check';
  }
  
  $form = array();
  //get the current stage
  $form = rawpheno_get_header($form, $form_state);
  
  //load form
  switch($form_state['stage']) {
    case 'check':
      //stage 01 - form drag and drop uploader
      return rawpheno_form_check($form, $form_state);
      break;

    case 'review':
      //stage 02 - describe form, optional, only when there is additional trait
      return rawpheno_form_review($form, $form_state);
      break;
	 
    case 'save':
      //stage 03 - success page
      return rawpheno_form_save($form, $form_state);
      break;
  }

  return $form;
}

/**
 * form stage 01 - upload spreadsheet and perfomr a
 * basic compliance test
 * 
 * @param type $form
 * @param type $form_state
 * @return type
 */
function rawpheno_form_check($form, &$form_state) {
  //create an instance of DragNDrop Upload
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
  
  //save spreadsheet - go to stage 03
  $form['stage01_submit_save'] = array(
    '#type' => 'submit',
    '#value' => 'Save spreadheet',
  );
  
  //describe trait - go to stage 02
  $form['stage01_submit_review'] = array(
    '#type' => 'submit',
    '#value' => 'Describe additional traits/column headers',
  );
  
  //close error window button
  $form['stage01_button_close'] = array(
    '#markup' => '<br /><a class="button" id="edit-stage01-button-close" href="javascript:void();">
                  Close error and try again</a>',
  );
  
  //attach required css and js
  $form['#attached']['css'] = array(drupal_get_path('module', 'rawpheno') . '/upload/css/rawpheno_upload_stage01.css');
  $form['#attached']['js'] = array(drupal_get_path('module', 'rawpheno') . '/upload/js/rawpheno_upload_stage01.js');

  return $form;  
}

/**
 * form stage 02 - review new traits found
 * allow user to describe and save a additional trait
 *
 * @param type $form
 * @param type $form_state
 * @return form
 */
function rawpheno_form_review($form, &$form_state) {
  $h = 'Error in checked column headers/traits. Please review form for any empty fields';
  $h = rawpheno_function_format($h, 'em');
  
  $form['stage02_window_error'] = array(
    '#markup' => '<div id="stage02-window-error" class="messages error">Required fields empty.<pre>'.$h.
                 '</pre></div>',
  );
  
  //container for each additonal column header found
  $form['xls_review_fldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Check the traits that you want to describe and save'),
  );
  
  //read all new column headers
  $new_header = variable_get('rawpheno_new_header');
  if (isset($new_header) && count($new_header) > 0) {
    ///create form elements to each trait
    foreach($new_header as $i => $k) {
      if (isset($k) && !empty($k)) {
        //fieldset to contain one set of fields of a trait
        $form['xls_review_fldset']['fldset_' . $i] = array(
          '#type' => 'fieldset',
          '#title' => '',
        );
          //all fields are required - no empty
          //CHECKBOX to let user select a trait to describe and save
          $form['xls_review_fldset']['fldset_' . $i]['chk_' . $i] = array(
            '#type' => 'checkbox',
            '#title' => t(ucwords($k)),
          );
          //TERM NAME/TRAIT/HEADER
          $form['xls_review_fldset']['fldset_' . $i]['txt_header_' . $i] = array(
            '#type' => 'hidden',
            '#value' => $k,
          );
          
          //these fields are hidden by default - show when check/selected
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
          //DESCRIPTION - describe the trait including the unit used
          $form['xls_review_fldset']['fldset_' . $i]['txtarea_describe_' . $i] = array(
            '#type' => 'textarea',
            '#title' => t('Describe the method used*'),
            '#description' => t('Describe the method used to collect this data if you used a scale, be specific'),
          );
          
          //required mark
          $form['xls_review_fldset']['fldset_' . $i]['required_mark'] = array(
            '#markup' => '<small>* means required</small>',
            '#suffix' => '</div>',
          );
      }
    } 
    ///
  
    //save spreadsheet - go to stage 03
    $form['stage02_submit_save'] = array(
      '#type' => 'submit',
      '#value' => 'Save spreadheet',
    );
  } 
  else {
    //no additional traits found or new header variable is empty
    //upload another spreadsheet button - go to stage 01
    $form['stage02_submit_save'] = array(
      '#type' => 'submit',
      '#value' => 'Upload another spreadsheet',
    );
  }
  
  //attach required css and js
  $form['#attached']['css'] = array(drupal_get_path('module', 'rawpheno') . '/upload/css/rawpheno_upload_stage02.css');
  $form['#attached']['js'] = array(drupal_get_path('module', 'rawpheno') . '/upload/js/rawpheno_upload_stage02.js');
  
  return $form;
}

/**
 * form stage 03 - final stage - save the spreadsheet to database
 *
 * @param type $form
 * @param type $form_state
 * @return type
 */
function rawpheno_form_save($form, &$form_state) {
  //display success message
  $h = 'Thank you! your file has been submitted';
  $h = rawpheno_function_format($h, 'em');
  
  $form['stage03_window_success'] = array(
    '#markup' => '<div id="stage03-window-success" class="messages status">Spreadsheet submitted<pre>'.$h.
                 '</pre></div><br />',
  );

  //upload another spreadsheet button - go to stage 01
  $form['stage03_submit_save'] = array(
    '#type' => 'submit',
    '#value' => 'Upload another spreadsheet',
  );
  
  //home button
  $form['stage03_submit_home'] = array(
    '#type' => 'submit',
    '#value' => 'Go to home KnowPulse',
  );
  
  //attach required css and js
  $form['#attached']['css'] = array(drupal_get_path('module', 'rawpheno') . '/upload/css/rawpheno_upload_stage03.css');

  return $form;
}