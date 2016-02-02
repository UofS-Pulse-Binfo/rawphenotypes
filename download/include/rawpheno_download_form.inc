<?php

/**
 * @file
 * Contains all implementations of hook_form().
 */
 
module_load_include('inc', 'rawpheno', 'download/include/rawpheno_download_submit');
module_load_include('inc', 'rawpheno', 'download/include/rawpheno_download_validate');

/**
 * Function callback for phenotypes/download
 *
 * @param type $form
 * @param type $form_state
 * @return type
 */
function rawpheno_download($form, &$form_state) {
  //Fieldset container for download form.
  $form['download_fldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Select location and traits that you want to download'),
    '#attributes' => array('id' => 'field-container'),
  );
  
  //Query all locations available.
  $d = db_query("SELECT DISTINCT t1.value, t1.value as location FROM {pheno_plantprop} as t1
                 INNER JOIN {chado.cvterm} as t2 ON t1.type_id = t2.cvterm_id
                 WHERE t2.name = 'location' ORDER BY t1.value ASC");
  
  $location = db_query($d)->fetchAllKeyed();
  
  //Primary select field - an AJAX chained select that updates secondary 
  //select field (traits) based on location selected. 
  //Default to othe first location in the list.
  $form['download_fldset']['location'] = array(
    '#title' => t('Location'),
    '#type' => 'select',
    '#options' => $location,
    '#default_value' => reset($location),
    '#multiple' => TRUE,
    '#size' => 15,
    '#prefix' => '<div class="frm-cell">',
    '#suffix' => '</div>',
    '#ajax' => array(
      'event' => 'change',
      'callback' => 'rawpheno_download_measurements',
      'wrapper' => 'div-measurements',
      'progress' => array('type'=>'', 'message'=>'')
    ),
  );
  
  //Div wrapper to display ajax result.
  $form['download_fldset']['wrapper'] = array(
    '#prefix' => '<div id="div-measurements">',
    '#suffix' => '</div>'
  );
  
  //Determine default traits to display by the value of location
  if (isset($form_state['values']['location'])) {
    //When user has selected a location.
    $loc = $form_state['values']['location'];
    $loc = "'".implode("','",$loc)."'";
  }
  else {
    //No selection made or on page load - default location to the first 
    //location available in primary select field.
    $loc = "'".reset($location)."'";
  }
  
  //When location is established, query all available traits per location.
  $t = db_query(sprintf("SELECT DISTINCT t1.type_id, t2.definition 
                 FROM {pheno_measurements} AS t1
                   INNER JOIN {chado.cvterm} AS t2 ON t1.type_id = t2.cvterm_id
                 WHERE t1.plant_id IN 
                   (SELECT DISTINCT plant_id FROM {pheno_plantprop} WHERE value IN (%s))
                   AND t1.value <> ''
                 ORDER BY t1.type_id ASC", $loc));   
  
  //Return trait id => trait definition array.                 
  $traits = db_query($t)->fetchAllKeyed();
    
  //Secondary select field - list of traits avaliable for download.
  $form['download_fldset']['wrapper']['traits'] = array(
    '#title' => count($traits).' Traits available',
    '#type' => 'select',
    '#options' => $traits,
    '#multiple' => TRUE,
    '#size' => 15,
  );
  
  //Submit button.
  $form['download_fldset']['download_submit_download'] = array(
    '#type' => 'submit',
    '#value' => 'Download',
    '#prefix' => '<div class="frm-cell"><input type="checkbox"> Select all traits',
    '#suffix' => '</div><div style="clear: both;"></div>',
  );
  
  //Attach CSS and JavaScript
  $form['#attached']['css'] = array(drupal_get_path('module', 'rawpheno') . '/download/css/rawpheno_download_style.css');
  $form['#attached']['js'] = array(drupal_get_path('module', 'rawpheno') . '/download/js/rawpheno_download_stage01.js');
  
  return $form;
}

/**
 * AJAX function callback in primary select (locations).
 *
 * @param type $form
 * @param type $form_state
 * @return array
 */
function rawpheno_download_measurements($form, $form_state) {
  return $form['download_fldset']['wrapper'];
}