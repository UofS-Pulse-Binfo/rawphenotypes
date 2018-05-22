<?php

/**
 * @file
 * Functions required in rawdata page.
 */

/**
 * Page callback: Displays a heat map chart.
 *
 * NOTE: The following column headers are excluded from the traits that can be visualized.
 *   Planting Date (date),
 *   Disease-specific Comments,
 *   Comments
 *   # of Seeds Planted (count)
 */
function rawpheno_rawdata($form, &$form_state) {
  // Navigation button. Related page of rawdata/summary page is download page.
  $form['page_button'] = array(
    '#type' => 'markup',
    '#markup' => t('Download Data &#10095;'),
  );

  // Query project that has data saved to it.
  $sql = "SELECT DISTINCT t1.project_id, t1.name
          FROM {project} AS t1 RIGHT JOIN pheno_plant_project AS t2 USING(project_id)
          WHERE plant_id IS NOT NULL
          ORDER BY t1.project_id ASC";

  $project = chado_query($sql)
    ->fetchAllKeyed();

  $form['rawdata_txt_project'] = array(
    '#type' => 'hidden',
    '#value' => implode(',', array_keys($project))
  );

  // Select project select box.
  $form['rawdata_sel_project'] = array(
    '#type' => 'select',
    '#title' => t('Select project and trait:'),
    '#options' => $project,
    '#id' => 'rawdata-sel-project',
  );

  // Select trait select box.
  // Get the cv_id of phenotype_measurement_types
  if (function_exists('chado_get_cv')) {
    $cv_id = chado_get_cv(array('name' => 'phenotype_measurement_types'));
  }
  else {
    $cv_id = tripal_get_cv(array('name' => 'phenotype_measurement_types'));
  }

  $sql = "SELECT t1.cvterm_id, t1.name
          FROM {cvterm} AS t1 INNER JOIN pheno_project_cvterm AS t2 USING(cvterm_id)
          WHERE
            t2.project_id = :project_id
            AND t1.cv_id = :cv_id
            AND t1.name NOT IN ('Plot', 'Entry', 'Name', 'Rep', 'Location', 'Planting Date (date)', 'Disease-specific Comments', 'Comments', '# of Seeds Planted (count)')
          ORDER BY t1.cvterm_id ASC";

  $default_option = array(0 => 'Select a trait to hightlight in the chart');

  // Create the select field and populate it with traits specific to a project.
  foreach(array_keys($project) as $p_id) {
    $q_string = array(':project_id' => $p_id, ':cv_id' => $cv_id->cv_id);
    $traits = chado_query($sql, $q_string)
      ->fetchAllKeyed();

    // List only traits that has actual data from pheno measurements table.
    foreach($traits as $id => $val) {
      $h = rawpheno_download_trait_has_data($id, '', $p_id);
      if ($h === FALSE) {
        unset($traits[$id]);
      }
    }

    $traits = $default_option + $traits;

    $form['sel_' . $p_id] = array(
      '#type' => 'select',
      '#title' => '&nbsp;',
      '#options' => $traits,
      '#default_value' => reset($traits),
      '#attributes' => array(
        'name' => 'rawdata-sel-trait'
      ),
      '#states' => array(
        'visible' => array(':input[name="rawdata_sel_project"]' => array('value' => $p_id)),
      ),
    );
  }

  // SVG elements.
  // SVG canvas.
  $form['page_content'] = array(
    '#type' => 'markup',
    '#markup' => '<svg class="data-chart">

      <defs>
        <filter id="dropshadow" x="0" y="0">
          <feOffset result="offOut" in="SourceAlpha" dx="0" dy="2" />
          <feGaussianBlur result="blurOut" in="offOut" stdDeviation="3" />
          <feBlend in="SourceGraphic" in2="blurOut" mode="normal" />
        </filter>
      </defs>

    </svg>',
  );

  // Hidden field containing url to JSON (summary data).
  $form['json_url'] = array(
    '#type' => 'hidden',
    '#value' => url('/'),
    '#attributes' => array('id' => array('rawdata-json'))
  );

  // Attach D3 JS library.
  $d3_lib = libraries_load('d3js');

  if (isset($d3_lib) && !empty($d3_lib['loaded'])) {
    $form['d3lib']['#attached']['libraries_load'][] = array('d3js');
  }

  // Attach CSS and JavaScript.
  $path = drupal_get_path('module', 'rawpheno') . '/theme/';
  $form['#attached']['css'] = array($path . 'css/rawpheno.rawdata.style.css');
  $form['#attached']['js'] = array($path . 'js/rawpheno.rawdata.script.js');

  return $form;
}
