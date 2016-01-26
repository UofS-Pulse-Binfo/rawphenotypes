<?php

/**
 * @file
 * implements hook submit in phenotypes/download page
 */
 
/**
 * download hook submit
 *
 * @param type $form
 * @param type $form_state
 */
function rawpheno_download_submit($form, &$form_state) {
  //which submit trigger submit action
  $btn_submit = $form_state['triggering_element']['#value'];   
  //direct user to the right page
  switch($btn_submit) {
    case RAWPHENO_DOWNLOAD_BTN_DOWNLOAD_02:
      //upload another spreadsheet
      drupal_goto('phenotypes/raw/download');
      break;
  
    case RAWPHENO_DOWNLOAD_BTN_HOME:
      //home KnowPulse
      drupal_goto('<front>');
      break;
  }
  
  //load next page or download page
  $next_stage = ($form_state['stage'] == 'form') ? 'download' : 'form';
  //rebuild form
  if(isset($form_state['multistep_values']['form_build_id'])) {
    $form_state['values']['form_build_id'] = $form_state['multistep_values']['form_build_id'];
  }
  
  $form_state['multistep_values'][$form_state['stage']] = $form_state['values'];
  $form_state['new_stage'] = $next_stage;
  
  $form_state['multistep_values']['form_build_id'] = $form_state['values']['form_build_id'];
  $form_state['stage'] = $form_state['new_stage'];
  $form_state['rebuild'] = TRUE;
}