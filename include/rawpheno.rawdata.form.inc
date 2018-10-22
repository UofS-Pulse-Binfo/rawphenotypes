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

  // The summarized list of cvterm_ids from MVIEW returned by inner most query will be passed to function that converts
  // comma separated values into individual values (cvterm_id numbers) and the result is the parameter of ANY clause
  // that will filter cvterms to only those in the list. Final rows are in JSON object and sorted alphabetically by name
  // that will be passed on to the select field of rawdata form.
  $sql_cvterm = "
    SELECT c_j.cvterm_json->>'id', c_j.cvterm_json->>'name' FROM (
      SELECT JSON_BUILD_OBJECT('id', cvterm_id, 'name', name) AS cvterm_json FROM {cvterm} WHERE cvterm_id = ANY ((
        SELECT STRING_TO_ARRAY(list_id.all_traits, ',') FROM (
          SELECT string_agg(DISTINCT all_traits, ',') AS all_traits
          FROM {rawpheno_rawdata_mview}
          WHERE plant_id IN (SELECT plant_id FROM pheno_plant_project WHERE project_id = :project_id)
          LIMIT 1
        ) AS list_id
      )::int[])
    ) AS c_j
    WHERE c_j.cvterm_json->>'name' NOT IN ('Rep', 'Entry', 'Location', 'Name', 'Plot', 'Planting Date (date)', '# of Seeds Planted (count)')
    ORDER BY c_j.cvterm_json->>'name' ASC
  ";

  // Add first option as instruction to this field.
  $default_option = array(0 => 'Select a trait to hightlight in the chart');

  // Create the select field and populate it with traits specific to a project.
  foreach(array_keys($project) as $p_id) {
    // Trait id numbers.
    $trait_ids = chado_query($sql_cvterm, array(':project_id' => $p_id))
      ->fetchAllKeyed();

    $traits_array = array_unique($trait_ids);
    $traits = $default_option + $traits_array;

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
