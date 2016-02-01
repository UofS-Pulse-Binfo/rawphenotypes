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
  $traits = $form_state['values']['traits'];
  if (count($traits) <= 0) {
    //no traits selected
    form_set_error('traits', t('Please select traits and try again.'));
  }
}