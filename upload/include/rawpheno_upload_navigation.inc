<?php

/**
 * @file
 * handles the navigation of the rawpheno stages.
 */

/**
 * generate a header indicating current stage.
 * 
 * @param type $form
 * @param type $form_state
 * @return type 
 */
function rawpheno_get_header($form, &$form_state) {
  //on first loat, initialize stage to stage 01 - check spreadsheet (uploader)
  $form_state['stage'] = isset($form_state['stage']) ? $form_state['stage'] : 1;
  
  //array of stages
  $form_stages = array('check' => 1, 'review' => 2, 'save' => 3);
  $current_step = (isset($form_stages[$form_state['stage']])) ? $form_stages[$form_state['stage']] : 1;
  
  //stage guide to user
  $stages = array(1 => '1. Upload Spreadsheet',
                  2 => '2. Describe Traits',
				          3 => '3. Save Spreadsheet');
				  
  $markup = '<div class="progress-container"><ol class="progress-meter">'; 
  foreach($stages as $k => $v) {
    $class = ($k <= $current_step) ? 'progress-point done' : 'progress-point todo';
    $markup .= '<li class="'.$class.'">'.$v.'</li>'; 
  }
  $markup .= '</ol></div>';
 
  $form['header'] = array(
    '#type' => 'fieldset',
    '#title' => '',
    '#value' => $markup
  );        

  return $form;
}

/**
 * given the current stage, calculate what the next step would be
 * 
 * @param type $form_state
 * @return string 
 */
function rawpheno_next_page($form, &$form_state) {
  //get the address of the next page based on the current stage
  switch($form_state['stage']) {
    case 'check':
      //stage check - go to save (stage 03) or review (stage 02)
      $btn_submit = $form_state['triggering_element']['#value'];
      return ($btn_submit == 'Save spreadheet') ? 'save' : 'review';
      break;

    case 'review':
      //stage review - go to save (stage 03)
      return 'save';
      break; 
  }
}