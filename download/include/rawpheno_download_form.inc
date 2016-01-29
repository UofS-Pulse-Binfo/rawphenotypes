<?php

/**
 * @file
 * implements hook form in phenotypes/download page
 */
 
module_load_include('inc', 'rawpheno', 'download/include/rawpheno_download_submit');
module_load_include('inc', 'rawpheno', 'download/include/rawpheno_download_validate');

/**
 * menu callback for rawpheno download
 *
 * @param type $form
 * @param type $form_state
 */
function rawpheno_download($form, &$form_state) {
  if (!isset($form_state['stage'])) {
    $form_state['stage'] = 'form';
  }
  
  $form = array();
  switch($form_state['stage']) {
    case 'form':
      //display form elements
      return rawpheno_download_traits_form($form, $form_state);
      break;

    case 'download':
      //generate csv and download 
      return rawpheno_download_autodownload_form($form, $form_state);
      break;
  }

  return $form;
}

/**
 * rawpheno download select traits form
 *
 * @param type $form
 * @param type $form_state
 * @return type
 */
function rawpheno_download_traits_form($form, $form_state) {
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
  
  //Primary select field with ajax option. 
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
      'progress' => array('type'=>'throbber', 'message'=>'')
    ),
  );
  
  //Div wrapper for ajax result.
  $form['download_fldset']['wrapper'] = array(
    '#prefix' => '<div id="div-measurements">',
    '#suffix' => '</div>'
  );
  
  //Populate location and traits fields.
  if (isset($form_state['values']['location'])) {
    //When user has selected a location.
    $loc = $form_state['values']['location'];
    $loc = implode("','",$loc);

    $traits = rawpheno_download_measurement_set($loc);
  }
  else {
    $loc = reset($location);
    //Default location to the first location available in primary select field.
    $traits = rawpheno_download_measurement_set($loc);
  }
  
  //Secondary select field showing all available traits.
  $form['download_fldset']['wrapper']['traits'] = array(
    '#title' => count($traits).' Traits available',
    '#type' => 'select',
    '#options' => $traits,
    '#multiple' => TRUE,
    '#size' => 35,

  );
  
  $form['download_fldset']['download_submit_download'] = array(
    '#type' => 'submit',
    '#value' => 'Download',
    '#prefix' => '<div class="frm-cell"><input type="checkbox"> Select all traits',
    '#suffix' => '</div><div style="clear: both;"></div>',
  );

  
  //attach css and js
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

/**
 * Query all available traits based on location selected
 *
 * @param string $location
 * @param string $trait
 * @return array
 */
function rawpheno_download_measurement_set($location) {
  $t = "SELECT DISTINCT t1.type_id AS trait_id, t3.definition AS trait
        FROM {pheno_measurements} AS t1 
          INNER JOIN {pheno_plantprop} AS t2 ON t1.plant_id = t2.plant_id
          INNER JOIN {chado.cvterm} AS t3 ON t1.type_id = t3.cvterm_id
        WHERE t2.value IN ('$location') 
          AND t1.value <> '' 
        ORDER BY t1.type_id ASC";   
  
  //Return trait id => trait definition array.                 
  return db_query($t)->fetchAllKeyed();
}

/**
 * rawpheno download autodownload CSV file
 *
 * @param type $form
 * @param type $form_state
 * @return type
 */
function rawpheno_download_autodownload_form($form, $form_state) {
  //delay download for 4 sec
  //sleep(4);
  
  //create csv file for download
  $location = $form_state['values']['location'];
  $location = implode("','",$location);
  
  //include plantprop traits
  $plantprop_traits = 'Plot,Entry,Name,Rep,Location';
  
  //list of traits to search
  $traits = $form_state['values']['traits'];
  $traits = implode(',', $traits);
  
  $t = db_query("SELECT ARRAY_TO_STRING(ARRAY_AGG(definition),',') 
                 FROM {chado.cvterm} 
                 WHERE cvterm_id IN ($traits)");
  
  $h = $plantprop_traits.','.$t->fetchField();
  
  dpm($h);
  
  /*
  //file  
  $file_name = 'rawpheno' . $location.'csv' . time() . '.csv';
  $file_csv = file_save_data($h.$raw, 'public://' . $file_name);
  
  //set header and save file
  $path = 'public://'.$file_name;
  
  $http_headers = array(
    'Content-Type' => 'text/csv',
    'Content-Disposition' => 'attachment; filename="' . 'rawdata.csv' . '"',
    'Content-Length' => filesize($path),
  );

  if (strpos($_SERVER['HTTP_USER_AGENT'], 'MSIE')) {
    $http_headers['Cache-Control'] = 'must-revalidate, post-check=0, pre-check=0';
    $http_headers['Pragma'] = 'public';
  } 
  else {
    $http_headers['Pragma'] = 'no-cache';
  }

  file_transfer($path, $http_headers);
  */
  
  return $form;
}