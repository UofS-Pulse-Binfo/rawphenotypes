<?php

/**
 * @file
 * Functions to manage navigation.
 */

/**
 * Function to generate a header indicating current stage.
 * 
 * @param $form
 * @param $form_state
 */
function rawpheno_get_header($form, &$form_state) {
  // On first load, initialize stage to stage 01 - check spreadsheet (uploader).
  $form_state['stage'] = isset($form_state['stage']) ? $form_state['stage'] : 1;
  
  // Array of stages. 
  $form_stages = array('check' => 1, 'review' => 2, 'save' => 3);
  $current_step = (isset($form_stages[$form_state['stage']])) ? $form_stages[$form_state['stage']] : 1;
  
  // Array of stage indicators.
  $stages = array(1 => '1. Validate Spreadsheet',
                  2 => '2. Describe New Trait',
				          3 => '3. Save Spreadsheet');
	
	$markup = '';
  foreach($stages as $k => $v) {
    $class = ($k <= $current_step) ? '' : ' progress-stage-todo';
    $markup .= '<div class="progress-stage' . $class . '" title="Need help?">
                  <span>&nbsp;</span>' . $v . '<span>&nbsp;</span>
                </div>';
  }

  // Add header to each stage with corresponding
  // stage information defined above.
  $form['header_upload'] = array(
    '#type' => 'markup',
    '#markup' => $markup,
  ); 
  
  return $form;
}


/**
 * Function to calculate the next stage.
 * 
 * @param $form
 * @param $form_state
 *
 * @return
 *   A string containing the stage name. 
 */
function rawpheno_next_page($form, &$form_state) {
  // Get the address/name of the next page based on the current stage.
  switch($form_state['stage']) {
    case 'check':
      // In stage check, next is stage 03 or stage 02.
      $btn_submit = $form_state['triggering_element']['#value'];
      return ($btn_submit == 'Save spreadheet') ? 'save' : 'review';
      break;

    case 'review':
      // In stage review, next is stage 03.
      return 'save';
      break; 
  }
}