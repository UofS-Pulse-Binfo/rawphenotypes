<?php

/**
 * @file
 * Implements hook_validate().
 */
function rawpheno_download_validate($form, &$form_state) {
  $traits = $form_state['values']['traits'];
  if (count($traits) <= 0) {
    // No trait select.
    form_set_error('traits', t('Please select traits and try again.'));
  }
}
