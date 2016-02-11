<?php

/**
 * Page callback: Displays a heat map chart.
 */
function rawpheno_rawdata($form, &$form_state) {
  // Container for svg canvas.
  $form['rawdata'] = array(
    '#markup' => '<div id="rawdata-container">
                    <svg class="data-chart"></svg>
                  </div>',    
  );
  
  // Hidden field containing url to json.
  $form['json_url'] = array(
    '#type' => 'hidden',
    '#value' => $GLOBALS['base_url'] . '/rawdata',
    '#attributes' => array('id' => array('rawdata-json'))
  );
  
  // Attach CSS and JavaScript.
  $path = drupal_get_path('module', 'rawpheno') . '/rawdata/';
  $form['#attached']['css'] = array($path . 'css/rawpheno_rawdata_style.css');
  $form['#attached']['js']  = array($path . 'js/rawpheno_rawdata_script.js');
  
  // Attach d3 JS library.
  $d3_lib = libraries_load('d3');
  if (isset($d3_lib) && !empty($d3_lib['loaded'])) {
    $form['#attached']['libraries_load'][] = array('d3');
  }
  
  return $form;
}