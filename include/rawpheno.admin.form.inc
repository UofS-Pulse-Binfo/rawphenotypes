<?php
/**
 * @file
 * Contains administrative interfaces for this module.
 */
function rawpheno_admin_interface($form, &$form_state) {
  $form['rawpheno_colour_scheme'] = array(
    '#type' => 'textfield',
    '#title' => t('Enter colour:'),
    '#default_value' => variable_get('rawpheno_colour_scheme'),
    '#size' => 40,
    '#maxlength' => 20,
    '#description' => t('eg. HEX: #304356, blue | <a href="@adobe-kuler" target="_blank">get more colours here</a>', 
                        array('@adobe-kuler' => url('https://color.adobe.com/'))),
    '#required' => TRUE,
  );
  
  // Headers and title
  $form['fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Page title'),
    '#collapsed' => FALSE,
  );
    
    // Rawdata page.
    $form['fieldset']['rawpheno_rawdata_title'] = array(
      '#type' => 'textfield',
      '#title' => t('Title of Rawdata page:'),
      '#default_value' => variable_get('rawpheno_rawdata_title'),
      '#size' => 120,
      '#maxlength' => 220,
      '#required' => TRUE,
    );
    
    // Download Page.
    $form['fieldset']['rawpheno_download_title'] = array(
      '#type' => 'textfield',
      '#title' => t('Title of Download page:'),
      '#default_value' => variable_get('rawpheno_download_title'),
      '#size' => 120,
      '#maxlength' => 220,
      '#required' => TRUE,
    );  
    
    // Instructions page.
    $form['fieldset']['rawpheno_instructions_title'] = array(
      '#type' => 'textfield',
      '#title' => t('Title of Instructions page:'),
      '#default_value' => variable_get('rawpheno_instructions_title'),
      '#size' => 120,
      '#maxlength' => 220,
      '#required' => TRUE,
    );  
  
    // Upload page.
    $form['fieldset']['rawpheno_upload_title'] = array(
      '#type' => 'textfield',
      '#title' => t('Title of Upload page:'),
      '#default_value' => variable_get('rawpheno_upload_title'),
      '#size' => 120,
      '#maxlength' => 220,
      '#required' => TRUE,
    );  
  
  return system_settings_form($form);
}