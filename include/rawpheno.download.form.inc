<?php
/**
 * @file
 * Contains download interface of this module.
 */

/**
 * Function callback: construct export data form.
 */
function rawpheno_download($form, &$form_state) {
  // Attach CSS and JavaScript
  $path = drupal_get_path('module', 'rawpheno') . '/theme/';
  $form['#attached']['css'] = array($path . 'css/rawpheno.download.style.css');

  // Navigation button. Related page of download page is rawdata/summary page.
  $form['page_button'] = array(
    '#type' => 'markup',
    '#markup' => t('View Summary &#10095;'),
  );

  // PROJECT SELECT BOX.
  if (isset($form_state['values']['sel_project'])) {
    // Project selected.
    $project_selected = $form_state['values']['sel_project'];
  }

  // Sort the project names according to Planting Date
  $sql = "SELECT project_id, name
          FROM {project} AS t1
            RIGHT JOIN pheno_plant_project AS t2 USING (project_id)
            LEFT JOIN pheno_measurements AS t3 USING (plant_id)
          WHERE t3.type_id = (SELECT cvterm_id FROM {cvterm} WHERE name = 'Planting Date (date)' LIMIT 1)
          GROUP BY project_id, name, t3.value
          ORDER BY t3.value DESC";

  $opt_project = chado_query($sql)
    ->fetchAllKeyed();

  if (count($opt_project) <= 0) {
    return $form;
  }
  else {
    $opt_project = array_unique($opt_project, SORT_REGULAR);
  }

  // AJAX wrapper.
  // Main wrapper
  $form['ajax_container'] = array(
    '#type' => 'markup',
    '#prefix' => '<div id="download-ajax-wrapper">',
    '#suffix' => '</div>',
  );

  // This a hidden field containing all project id.
  // This field will allow callback functions to get all project ids which is
  // the equivalent of the option select all project from the project select box.
  $form['ajax_container']['txt_project'] = array(
    '#type' => 'hidden',
    '#value' => implode(',', array_keys($opt_project)),
  );

  $form['ajax_container']['sel_project'] = array(
    '#type' => 'select',
    '#title' => t('Project'),
    '#options' => $opt_project,
    '#multiple' => FALSE,
    '#id' => 'download-sel-project',
    '#ajax' => array(
      'event' => 'change',
      'callback' => 'rawpheno_download_get_locations_traits',
      'wrapper' => 'download-ajax-wrapper',
      'progress' => array('type' => '', 'message' => '')
    ),
  );

  // This will reset the project select box on load and page refresh.
  drupal_add_js('jQuery(document).ready(function() {
    jQuery("#download-sel-project").val(0);
  })', 'inline');

  // Define the project ids required by the next field.
  if (isset($project_selected)) {
    // When a project is selected. Default to the project selected.
    $project_id = $project_selected;
  }
  else {
    // No project select. This is the default to the first project.
    $p = array_keys($opt_project);
    $project_id = reset($p);
  }

  // LOCATION SELECT BOX.
  $sql = "SELECT DISTINCT value, value AS prj_location
          FROM pheno_plantprop
          WHERE
            type_id = (SELECT cvterm_id FROM {cvterm} WHERE name = 'Location' LIMIT 1) AND
            plant_id IN (SELECT plant_id FROM pheno_plant_project WHERE project_id IN (:project_id))
          ORDER BY value ASC";

  $opt_location = chado_query($sql, array(':project_id' => $project_id))
    ->fetchAllKeyed();

  $form['ajax_container']['sel_location'] = array(
    '#type' => 'select',
    '#title' => t('Select Location'),
    '#options' => $opt_location,
    '#multiple' => TRUE,
    '#size' => 7,
    '#id' => 'download-sel-location',
    '#ajax' => array(
      'event' => 'change',
      'callback' => 'rawpheno_download_get_traits',
      'wrapper' => 'download-ajax-wrapper-traits',
      'progress' => array('type' => '', 'message' => '')
    ),
  );

  $form['ajax_container']['chk_select_all_locations'] = array(
    '#title' => t('Select all Locations'),
    '#type' => 'checkbox',
    '#default_value' => 0,
    '#ajax' => array(
      'event' => 'change',
      'callback' => 'rawpheno_download_get_locations_traits',
      'wrapper' => 'download-ajax-wrapper',
      'progress' => array('type' => '', 'message' => '')
    ),
    '#id' => 'chk-select-all-locations',
  );

  $location_id = $opt_location;

  // Manage environment data file option.
  // Allow option when a project is selected and project and location combination
  // returns an environment data file.
  $add_option = FALSE;

  if (isset($project_selected) && $project_selected > 0) {
    if (isset($form_state['values']['sel_location'])
        && count($form_state['values']['sel_location']) > 0) {

      $location = $form_state['values']['sel_location'];

      $envfile = rawpheno_function_getenv($project_selected, $location);
      if ($envfile) {
        $add_option = TRUE;
      }
    }
  }

  drupal_add_js(array('rawpheno' => array('envdata_option' => $add_option)), array('type' => 'setting'));

  // TRAITS.
  // Select traits wrapper.
  $form['ajax_container']['ajax_container_traits'] = array(
    '#type' => 'markup',
    '#prefix' => '<div id="download-ajax-wrapper-traits">',
    '#suffix' => '</div>',
  );

  // Get traits given a location and project.
  $sql = "SELECT cvterm_id, name
          FROM {cvterm}
            RIGHT JOIN pheno_measurements AS t1 ON cvterm_id = type_id
          WHERE
            t1.value IS NOT NULL
            AND name NOT IN ('Planting Date (date)', '# of Seeds Planted (count)')
            AND plant_id IN (SELECT plant_id
              FROM pheno_plantprop INNER JOIN pheno_plant_project USING(plant_id)
              WHERE value IN (:location) AND project_id IN (:project))
          GROUP BY cvterm_id, name
          ORDER BY name ASC";

  $opt_trait = chado_query($sql, array(
    ':location' => $location_id,
    ':project' => $project_id
  ))
    ->fetchAllKeyed();

  $form['ajax_container']['ajax_container_traits']['sel_trait'] = array(
    '#type' => 'select',
    '#title' => t('Select Trait - @trait_count Traits available', array('@trait_count' => count($opt_trait))),
    '#options' => $opt_trait,
    '#multiple' => TRUE,
    '#size' => 15,
    '#id' => 'download-sel-trait',
  );

  $form['ajax_container']['chk_select_all_traits'] = array(
    '#title' => t('Select all Traits'),
    '#type' => 'checkbox',
    '#default_value' => 0,
    '#id' => 'chk-select-all-traits',
  );

  $form['div_buttons'] = array(
    '#prefix' => '<div id="div-button">',
    '#suffix' => '</div>',
  );

  $form['div_buttons']['chk_envdata'] = array(
    '#title' => t('<img src="@img" alt="Include Environment Data" title="Include Environment Data" align="absmiddle" /> (Include Environment Data)', array('@img' => '../../' . $path . 'img/env.gif')),
    '#type' => 'checkbox',
    '#default_value' => 0,
    '#id' => 'chk-envdata',
  );

  $form['div_buttons']['chk_rfriendly'] = array(
    '#title' => t('<img src="@img" alt="Make R Friendly" title="Make R Friendly" align="absmiddle" /> (Make R Friendly)', array('@img' => '../../' . $path . 'img/r.gif')),
    '#type' => 'checkbox',
    '#default_value' => 0,
  );


  $form['div_buttons']['download_submit_download'] = array(
    '#type' => 'submit',
    '#value' => 'Download',
  );

  $form['#attached']['js']  = array($path . 'js/rawpheno.download.script.js');

  return $form;
}


/**
 * Function callback: AJAX update location and traits select boxes when project is selected.
 */
function rawpheno_download_get_locations_traits($form, $form_state) {
  return $form['ajax_container'];
}


/**
 * Function callback: AJAX update trait select box.
 */
function rawpheno_download_get_traits($form, $form_state) {
  $location = $form_state['values']['sel_location'];
  $project = $form_state['values']['sel_project'];

  // Determine if the selected project is all project.
  if ($project == 0) {
    // Yes, then read the value of the hidden field containing project ids.
    $t = $form_state['values']['txt_project'];
    $project = explode(',', $t);
  }

  // Get all traits given a location and project.
  $opt_trait = rawpheno_download_load_traits($location, $project);

  // Update the #options value of select a trait select box.
  $form['ajax_container']['ajax_container_traits']['sel_trait']['#options'] = $opt_trait;
  // Update the title.
  $form['ajax_container']['ajax_container_traits']['sel_trait']['#title'] = t('@count_trait Traits available', array('@count_trait' => count($opt_trait)));

  return $form['ajax_container']['ajax_container_traits'];
}


/**
 * Implements hook_validate().
 */
function rawpheno_download_validate($form, &$form_state) {
  if (!$form_state['values']['sel_location']) {
    form_set_error('sel_location', t('No location(s) selected. Please select location(s) and try again'));
  }

  if (!$form_state['values']['sel_trait']) {
    form_set_error('sel_trait', t('No trait(s) selected. Please select trait(s) and try again'));
  }
}


/**
 * Implements hook_form_submit().
 *
 * Generate a comma separated values (csv) file based on the location and trait set selected.
 */
function rawpheno_download_submit($form, &$form_state) {
  // Project select field.
  // Project by default is 0 - all projects then we want all project id field.
  // This is field is never an array.
  $prj = $form_state['values']['sel_project'];
  $all_prj = $form_state['values']['txt_project'];
  $prj = ($prj == 0) ? $all_prj : $prj;

  // Location select field.
  // Location select field is an empty array - all locations.
  // Otherwise, it will be an associative array where location is both key and value.
  // Convert this to comma separated string when there's anything else set to 0 - for all locations.
  $loc = $form_state['values']['sel_location'];
  $loc = (count($loc) > 0) ? implode(',', $loc) : 0;

  // Trait select field.
  // Trait select field is an empty array - all traits.
  // Otherwise, it will be an associative array where trait is both key and value.
  // Convert this to comma separated string when there's anything else set to 0 - for all traits.
  $trt = $form_state['values']['sel_trait'];
  $trt = (count($trt) > 0) ? implode(',', $trt) : 0;

  // Lastly, if user wants Environment Data and R version.
  $env = $form_state['values']['chk_envdata'];
  $rvr = $form_state['values']['chk_rfriendly'];

  // Construct environment data files archive.
  $env_filename = 0;

  // Due to unable to execute shell_exec/tar in tripal download api -
  // This block will fetch all environment data files when available for
  // a given project and location combination. Once files are ready, tar
  // them together when more than 1 files returned otherwise, use single file.
  // The filename for either case will be forwarded to tripal download api
  // wich will create a markup for a secondary download link.
  // See: rawpheno_trpdownload_summarize_download()

  // In this method, tripal download api will be solely responsible for
  // generating the rawphenotypic data file while hook_submit is for env files.

  if (isset($env) && $env == 1) {
    // Ensure that project and location combination return an environment data file.
    $project = explode(',', $prj);
    $location = explode(',', $loc);

    $files = rawpheno_function_getenv($project, $location);

    if (count($files) > 0) {
      // Env file available.
      $envs = array();

      foreach($files as $file) {
        $envs[] = $file->filename;
      }

      if (count($envs) == 1) {
        // Single env file found. Fetch the file (xlsx usually) and submit to tripal download.
        $env_filename = reset($envs);
      }
      else {
        // Multiple env files found. Fetch all files, tar (archive) and submit to tripal download.
        $public = drupal_realpath('public://');
        $tar_filename = 'environment_data_' . date('ymdis') . '.tar';
        $tar_file = $public . '/' . $tar_filename;

        $tar_cmd = 'tar -cf ' . escapeshellarg($tar_file) . ' -C ' . escapeshellarg($public) . ' ';
        $tar_cmd .= implode(' ', $envs) . ' 2>&1';

        // Package everything...
        shell_exec($tar_cmd);
        $env_filename = $tar_filename;
      }
    }
  }

  // Contain all query parameters/string into one string.
  // Decode first when reading this string using base64_decode() function.
  $url = 'p=' . $prj . '&l=' . $loc . '&t=' . $trt . '&r=' . $rvr . '&e=' . $env . '&file=' . $env_filename;

  // Format url for redirect.
  $form_state['redirect'] = array(
    '/phenotypes/raw/csv',
    array(
      'query' => array(
        'code' => base64_encode($url),
      ),
    ),
  );
}
