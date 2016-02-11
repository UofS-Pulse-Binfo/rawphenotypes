<?php

/**
 * @file
 * Contains all implementations of hook_form().
 */

// Include implementations of hook_submit() and hook_validate(). 
module_load_include('inc', 'rawpheno', 'download/include/rawpheno_download_submit');
module_load_include('inc', 'rawpheno', 'download/include/rawpheno_download_validate');

/**
 * Function callback: construct export data form.
 */
function rawpheno_download($form, &$form_state) {
  // Fieldset container for download form.
  $form['download_fldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Select locations and traits that you want to download'),
    '#attributes' => array('id' => 'field-container'),
  );
  
  // Query all locations available.
  $location = db_query("SELECT DISTINCT t1.value, t1.value AS location 
    FROM {pheno_plantprop} AS t1
      INNER JOIN {chado.cvterm} AS t2 ON t1.type_id = t2.cvterm_id
    WHERE t2.name = 'location' 
    ORDER BY t1.value ASC")
    ->fetchAllKeyed();
  
  // Primary select field - an AJAX chained select that updates secondary 
  // select field (traits) based on location selected. 
  // Default to the first location in the list.
  $form['download_fldset']['location'] = array(
    '#title' => t('Location'),
    '#type' => 'select',
    '#options' => $location,
    '#default_value' => reset($location),
    '#multiple' => TRUE,
    '#size' => 17,
    '#prefix' => '<div class="frm-cell">',
    '#suffix' => '</div>',
    '#ajax' => array(
      'event' => 'change',
      'callback' => 'rawpheno_download_measurements',
      'wrapper' => 'div-measurements',
      'progress' => array('type' => '', 'message' => '')
    ),
  );
  
  // Div wrapper to display ajax result.
  $form['download_fldset']['wrapper'] = array(
    '#prefix' => '<div id="div-measurements">',
    '#suffix' => '</div>'
  );
  
  // Determine default traits to display based on value of location.
  if (isset($form_state['values']['location'])) {
    // When user has selected a location.
    $loc = $form_state['values']['location'];
  }
  else {
    // No selection made or on page load - default location to the first 
    // location available in primary select field.
    $loc = reset($location);
  }
  
  // Query all traits available per location.
  // Note: planting date is excluded from the list in the same manner it is
  // excluded from the rawdata heat map.
  $traits = db_query("SELECT DISTINCT t1.type_id, t2.definition 
    FROM {pheno_measurements} AS t1
      INNER JOIN {chado.cvterm} AS t2 ON t1.type_id = t2.cvterm_id
    WHERE t1.plant_id IN 
      (SELECT DISTINCT plant_id FROM {pheno_plantprop} WHERE value IN (:location))
      AND t1.value <> ''
      AND t2.name <> 'planting_date'
    ORDER BY t1.type_id ASC", array(':location' => $loc))
    ->fetchAllKeyed();
    
  // Secondary select field - list of traits avaliable for download.
  $form['download_fldset']['wrapper']['traits'] = array(
    '#title' => count($traits).' Traits available',
    '#type' => 'select',
    '#options' => $traits,
    '#multiple' => TRUE,
    '#size' => 17,
  );
  
  // Submit button.
  $form['download_fldset']['download_submit_download'] = array(
    '#type' => 'submit',
    '#value' => 'Download',
    '#prefix' => '<div class="frm-cell"><input type="checkbox"> Select all traits',
    '#suffix' => '</div><div style="clear: both;"></div>',
  );
  
  // Attach CSS and JavaScript
  $path = drupal_get_path('module', 'rawpheno') . '/download/';
  $form['#attached']['css'] = array($path . 'css/rawpheno_download_style.css');
  $form['#attached']['js']  = array($path . 'js/rawpheno_download_stage01.js');
  
  return $form;
}

/**
 * Function callback: set form wrapper for AJAX.
 */
function rawpheno_download_measurements($form, $form_state) {
  return $form['download_fldset']['wrapper'];
}