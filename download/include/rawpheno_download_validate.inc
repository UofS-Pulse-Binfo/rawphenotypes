<?php

/**
 * @file
 * implements hook validate in phenotypes/download page
 */
 
/**
 * download hook validate
 *
 * @param type $form
 * @param type $form_state
 */
function rawpheno_download_validate($form, &$form_state) {
  if ($form_state['stage'] == 'form') {
    $location = $form_state['values']['location'];
    $select = 'sel-'.$location;
    $traits = $form_state['values'][$select];
  
    if (count($traits) <= 0) {
      //no traits selected
      form_set_error($select, t('Please select traits and try again.'));
    }
  }
}