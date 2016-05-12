<?php
/**
 * @file
 * Contains administrative interfaces for this module.
 */
 
/**
 * Function callback: construct administrative interface to modify page title and colour scheme.
 */
function rawpheno_admin_page_interface($form, &$form_state) {
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

// Include function to manage column headers, cv terms and variable names.
module_load_include('inc', 'rawpheno', 'include/rawpheno.function.measurements');

/**
 * Function callback: construct administrative interface to manage R Friendly transformation of column headers.
 */
function rawpheno_admin_headers_interface($form, &$form_state) {
  // Fieldset to contain R-Friendly transformation form.
  $form['fieldset_generate'] = array(
    '#type' => 'fieldset',
    '#title' => t('Generate R-Friendly Column Header'),
    '#collapsed' => FALSE,
    '#collapsible' => TRUE,
  );

    // Get all cvterms under phenotype measurement type.
    $sql = "SELECT t2.cvterm_id AS cvid, t2.name AS cvterm, t3.value AS rfriendly FROM 
      chado.cv AS t1 INNER JOIN chado.cvterm AS t2 USING (cv_id)
      INNER JOIN chado.cvtermprop AS t3 ON t2.cvterm_id = t3.cvterm_id  
      WHERE t1.name = 'phenotype_measurement_types' ORDER BY t2.cvterm_id ASC";
 
    $terms = db_query($sql)
      ->fetchAllKeyed(0, 1);
 
    $form['fieldset_generate']['select_trait'] = array(
      '#title' => '',
      '#type' => 'select',
      '#options' => array(0 => 'Select a term') + $terms,
      '#default_value' => reset($terms),
      '#ajax' => array(
        'callback' => 'ajax_rawpheno_modify_trait_callback',
          'wrapper' => 'div-modify',
          'effect' => 'fade',
        )
      );

    $form['fieldset_generate']['form_container'] = array(
      '#type' => 'markup',
      '#prefix' => '<div id="div-modify" style="display: none;">',
      '#suffix' => '</div>',
    );
 
    // Text field and a save button to save R Friendly term.
    $form['fieldset_generate']['form_container']['txt_rfriendly'] = array(
      '#type' => 'textfield',
      '#title' => t('R Friendly'),
      '#size' => 120,
      '#maxlength' => 220,
      '#required' => FALSE,
    );

    $form['fieldset_generate']['form_container']['save_term'] = array(
      '#type' => 'submit',
      '#value' => 'Save',
    );
  
   // Fieldset to contain R-Friendly transformation rules form.
  $form['fieldset_rules'] = array(
    '#type' => 'fieldset',
    '#title' => t('R-Friendly Transformation Rules'),
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
  ); 
  
    $form['fieldset_rules']['rawpheno_admin_transform_words'] = array(
      '#type' => 'textarea',
      '#title' => t('List of words to remove:'),
      '#default_value' => variable_get('rawpheno_admin_transform_words'),
      '#description' => 'Separate words with commas',
    );

    $form['fieldset_rules']['rawpheno_admin_transform_characters'] = array(
      '#type' => 'textarea',
      '#title' => t('List of characters to remove:'),
      '#default_value' => variable_get('rawpheno_admin_transform_characters'),
      '#description' => 'Separate characters with commas',
    );  
  
    $form['fieldset_rules']['rawpheno_admin_transform_replace'] = array(
      '#type' => 'textarea',
      '#title' => t('Match and replace:'),
      '#default_value' => variable_get('rawpheno_admin_transform_replace'),
      '#description' => 'Separate match and replace pairs with commas',
    );
    
    $form['fieldset_rules']['save_rules'] = array(
      '#type' => 'submit',
      '#value' => 'Save Rules',
    );
  
  // Fieldset to contain listing of column headers.
  $form['fieldset_list'] = array(
    '#type' => 'fieldset',
    '#title' => t('@count Column headers', array('@count' => count($terms))),
    '#collapsed' => FALSE,
    '#collapsible' => TRUE,
  );
    
    $terms = db_query($sql);
    // List all cvterm with corresponding R friendly version.
    $markup = '<table class="sticky-enabled table-select-processed tableheader-processed sticky-table">
      <thead><tr><th>Column Header</th> <th>R Friendly</th></tr></thead><tbody>';
    
    // List all traits and corresponding R Friendly transformation.
    $i = 0;
    foreach($terms as $term) {
      $class = ($i % 2) ? 'td_highlight' : '';
      $markup .= '<tr><td class="' . $class . '">' . $term->cvterm . '</td><td class="' . $class . '">';
      $markup .= (empty($term->rfriendly)) ? 'Not set' : $term->rfriendly;  
      $markup .= '</td></tr>';
    
      $i++;
    }
    $markup .= '</tbody></table>';
 
    $form['fieldset_list']['all_columnheaders'] = array(
      '#type' => 'markup',
      '#markup' => $markup
    );
    
  // Attach css and javascript.  
  $path = drupal_get_path('module', 'rawpheno') . '/theme/';
  $form['#attached']['css'] = array($path . 'css/rawpheno.style.css');
  
  return $form;
}

/**
* Function callback: construct form to allow admin generate R Friendly transformation.
*/
function ajax_rawpheno_modify_trait_callback($form, $form_state) {
  $selected_trait = $form_state['values']['select_trait'];
  
  // Get the R compatible trait from cvtermprop.
  $sql = "SELECT t1.name AS cvterm, t2.value AS rfriendly 
    FROM chado.cvterm as t1 inner join chado.cvtermprop AS t2 USING(cvterm_id)
    WHERE t1.cvterm_id = :cvterm_id AND 
    t2.cvterm_id = :cvterm_id 
    LIMIT 1";
  
  $term = db_query($sql, array(':cvterm_id' => $selected_trait));
  
  $rfriendly = $term->fetchField(1);
  
  if (empty($rfriendly)) {
    // No R compatible version of the trait.
    // Try to transform the trait to R friendly.
    $term = db_query($sql, array(':cvterm_id' => $selected_trait));
    $r_compatible = rawpheno_function_make_r_compatible($term->fetchField(0)); 
  } else {
    // R compatible is present, load to form field.
    $r_compatible = $rfriendly;  
  }
  
  // 4. Finally, replace spaces with dots.
  $form['fieldset_generate']['form_container']['txt_rfriendly']['#value'] = $r_compatible;
  
  // #AJAX update form field container with r friendly term.
  return $form['fieldset_generate']['form_container'];
}

/**
* Implements hook_submit().
* Save R Friendly transformation rules and generated terms.
*/
function rawpheno_admin_headers_interface_submit($form, &$form_state) {
  // Get the submit button clicked.
  $submit_btn = $form_state['triggering_element']['#value'];
  
  // Determine which save button was fired.
  if ($submit_btn == 'Save Rules') {
    // Save transformation rules.
    $words = $form_state['values']['rawpheno_admin_transform_words'];  
    variable_set('rawpheno_admin_transform_words', $words);  
    
    $chars = $form_state['values']['rawpheno_admin_transform_characters'];  
    variable_set('rawpheno_admin_transform_characters', $chars);  
    
    $replace = $form_state['values']['rawpheno_admin_transform_replace'];
    variable_set('rawpheno_admin_transform_replace', $replace);  
  }
  else if($submit_btn == 'Save') {
    // Save R compatible term.
    $cvterm_id = $form_state['values']['select_trait'];
    $rfriendly = $form_state['values']['txt_rfriendly'];
    
    db_update('chado.cvtermprop')
      ->fields(array(
        'value' => $rfriendly
      )) 
      ->condition('cvterm_id', $cvterm_id, '=')
      ->execute();
  }
}