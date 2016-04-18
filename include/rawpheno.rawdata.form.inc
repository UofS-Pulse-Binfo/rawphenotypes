<?php

/**
 * Page callback: Displays a heat map chart.
 */
function rawpheno_rawdata($form, &$form_state) {
  // Navigation button. Related page of rawdata/summary page is download page.
  $form['page_button'] = array(
    '#type' => 'markup',
    '#markup' => t('Download Data &#10095;'),
  );
 
  // Page Contents.
  // SVG canvas.
  $form['page_content'] = array(
    '#type' => 'markup',
    '#markup' => '<svg class="data-chart">
      <defs>
        <filter id="dropshadow" x="0" y="0" width="170%" height="200%">
          <feOffset result="offOut" in="SourceAlpha" dx="12" dy="9" />
          <feGaussianBlur result="blurOut" in="offOut" stdDeviation="7" />
          <feBlend in="SourceGraphic" in2="blurOut" mode="normal" />
        </filter>
      </defs>
    </svg>',
  );
  
  // Allow users to filter the chart based on traits.
  // List all traits available.
  
  // Get the cv_id of phenotype_measurement_types
  $cv_id = tripal_get_cv(array('name' => 'phenotype_measurement_types'));
  
  // Get all traits under phenotypes measurements type.
  $traits = db_query("SELECT cvterm_id, name 
    FROM chado.cvterm
    WHERE name <> 'Planting Date (date)' AND cv_id = :cv_id
    ORDER BY cvterm_id ASC", array(':cv_id' => $cv_id->cv_id))
    ->fetchAllKeyed();  

  $traits = array(0 => 'Select trait to highlight in the chart') + $traits;

  $form['select_trait'] = array(
    '#title' => '',
    '#type' => 'select',
    '#options' => $traits,
    '#default_value' => reset($traits),
  );
  
  // Hidden field containing url to JSON.
  $form['json_url'] = array(
    '#type' => 'hidden',
    '#value' => url('/rawdata'),
    '#attributes' => array('id' => array('rawdata-json'))
  );
  
  // Attach D3 JS library.
  $d3_lib = libraries_load('d3');
  if (isset($d3_lib) && !empty($d3_lib['loaded'])) {
    $form['#attached']['libraries_load'][] = array('d3');
  }
  
  // Attach CSS and JavaScript.
  $path = drupal_get_path('module', 'rawpheno') . '/theme/';
  $form['#attached']['css'] = array($path . 'css/rawpheno.rawdata.style.css');
  $form['#attached']['js'] = array($path . 'js/rawpheno.rawdata.script.js');
  
  return $form;
}