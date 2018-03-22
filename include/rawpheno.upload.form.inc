<?php

/**
 * @file
 * Contains the upload raw phenotypic data form.
 */

// Additional Helper functions
module_load_include('inc', 'rawpheno', 'include/rawpheno.upload.helpers');
// Include functions required in processing spreadsheet file.
module_load_include('inc', 'rawpheno', 'include/rawpheno.upload.excel');
module_load_include('inc', 'rawpheno', 'include/rawpheno.validation');

/**
 * Master form constructor which calls an individual form for each stage.
 *
 * @see rawpheno_element_validate()
 * @see rawpheno_element_submit()
 */
function rawpheno_upload_form_master($form, &$form_state) {
  // Navigation button. Related page of upload page is instructions page.
  $form['page_button'] = array(
    '#type' => 'markup',
    '#markup' => t('Standard Procedure &#10095;'),
  );

  // If the stage is not set then default to the first stage (i.e. 'check' )
  if (!isset($form_state['stage'])) {
    $form_state['stage'] = 'check';
  }

  // If a job_id was provided in the URL then the user wants information
  // on a prvious bulk loading job. Thus we should show them the last step.
  if (isset($form_state['build_info']['args'][0])) {
    $form_state['stage'] = 'save';
  }

  // Add the stage tracker/header.
  $form = rawpheno_get_header($form, $form_state);
  // Holds the current stage.
  $form_stage = $form_state['stage'];

  // Stage indicator for theme function.
  $form['current_stage'] = array(
    '#type' => 'value',
    '#value' => $form_stage,
  );

  // Add the next button for all but the last step.
  if ($form_stage != 'save') {
    $form['next_step'] = array(
      '#type' => 'submit',
      '#value' => 'Next Step',
      '#weight' => 100
    );
  }

  // Create a select box containing projects available - these are projects
  // that have associated column header set and must have at least 1 essential column header.
  // The projects are filtered to show only projects assigned to user.
  if ($form_stage != 'save') {
    $my_project = rawpheno_function_user_project($GLOBALS['user']->uid);

    if (count($my_project) > 0) {
      // When there is more than 1 project assigned to user, tell user to select a project
      // otherwise default to the only project available.
      if (count($my_project) > 1) {
        $my_project = array(0 => 'Please select a project') + $my_project;
      }

      // Default Project in project selector field.
      if (isset($_POST['sel_project']) AND $_POST['sel_project'] > 0) {
        // HTTP method post has the project id.
        $default_value = $_POST['sel_project'];
      }
      elseif (isset($form_state['values']['sel_project'])) {
        // Form state has the project id.
        $default_value = $form_state['values']['sel_project'];
      }
      else {
        // Neither has the project id number default to the first project in the project list.
        $default_value = 0;
      }

      // Determine if the project select box should be enabled.
      // Disabled in all stages but stage 01.
      $disabled = ($form_stage == 'check') ? FALSE : TRUE;

      $form['sel_project'] = array(
        '#type' => 'select',
        '#options' => $my_project,
        '#default_value' => $default_value,
        '#disabled' => $disabled,
        '#id' => 'rawpheno-select-project-field',
      );

      // This block is to ensure select project select box is always default to
      // "please select a project" or to the first project option in the beginning of the upload process.
      // It will also default to this option when user refreshes the page.
      if ($form_stage == 'check') {
        drupal_add_js('jQuery(document).ready(function() {
          jQuery("#rawpheno-select-project-field").val(0);
        })', 'inline');
      }
    }
    else {
      // No project is assigned to user.
      $form['no_project'] = array(
        '#markup' => '<div class="messages warning">' . t('No project is assigned to this account. Please contact the administrator of this website.') . '</div>',
      );
    }
  }

  // Note:
  // When there is no project defined in the module, the error message is handled by the theme.

  // Get the directory path of rawpheno module.
  $path = drupal_get_path('module', 'rawpheno') . '/theme/';

  // Load corresponding function callback together with JavaScript and CSS.
  switch($form_stage) {
    case 'check':
      // Stage 01 - upload and check spreadsheet.
      $form['#attached']['css'] = array($path . 'css/rawpheno.upload.stage01.css');
      $form['#attached']['js']  = array($path . 'js/rawpheno.upload.stage01.js',
                                        $path . 'js/rawpheno.upload.script.js');

      $form = rawpheno_upload_form_stage_check($form, $form_state);
      break;

    case 'review':
      // Stage 02 - describe form (only when there is additional trait).
      $form['#attached']['css'] = array($path . 'css/rawpheno.upload.stage02.css');
      $form['#attached']['js']  = array($path . 'js/rawpheno.upload.script.js');

      $form = rawpheno_upload_form_stage_review($form, $form_state);
      break;

    case 'save':
      // Stage 03 - save to databse and success page.
      $form['#attached']['css'] = array($path . 'css/rawpheno.upload.stage03.css');
      $form['#attached']['js']  = array($path . 'js/rawpheno.upload.stage03.js',
                                        $path . 'js/rawpheno.upload.script.js');

      $form = rawpheno_upload_form_stage_save($form, $form_state);
      break;
  }

  return $form;
}


/**
 * Function callback: Construct form for Stage 01.
 *
 * Stage 01 form allows user to upload data collection spreadsheet and perform basic compliance test.
 */
function rawpheno_upload_form_stage_check($form, &$form_state) {
  // Create an instance of DragNDrop Upload.
  // SETTINGS:
  //  #file_upload_max_size: max file size allowed
  //  #upload_location:      destination of file
  //  #upload_event:         manual - show an upload button or auto - uploads after drag drop
  //  #upload_validators:    allowed file extensions
  //  #upload_button_text:   label of upload button
  //  #droppable_area_text:  text in drop area
  //  #progress_indicator:   none, throbber or bar
  //  #progress_message:     message to display while processing
  //  #allow_replace:        allow user to replace file by drag and drop another file
  //  #standard_upload:      show browse button or not
  //  #upload_button_text:   submit button text (not required when auto submit is auto)

  $form['dnd'] = array(
    '#type' => 'dragndrop_upload',
    '#file_upload_max_size' => '10M',
    '#upload_location' => 'public://',
    '#upload_event' => 'auto',
    // NOTE: Accept the listed file extension and let the spreadsheet reader tell if file is valid to generate an an error message.
    //       No silent treatment.
    '#upload_validators' => array(
      'file_validate_extensions' => array('xlsx xls jpg jpeg gif png txt doc pdf ppt pps odt ods odp csv'),
    ),
    '#droppable_area_text' => t('Drag your Microsoft Excel Spreadsheet file here'),
    '#progress_indicator' => 'throbber',
    '#progress_message' => 'Validating your spreadsheet file. Please wait...',
    '#allow_replace' => 1,
    '#standard_upload' => 1,
    '#upload_button_text' => '',

    // We are adding our own element process function so that we can make a successfully
    // uploaded/validated file permanent during the AJAX process rather than waiting for
    // them to click the "Next" button.
    '#process' => array(
      'file_managed_file_process',
      'dragndrop_upload_element_element_process',
      'rawpheno_phenotype_upload_file_element_process',
    ),
  );

  return $form;
}


/**
 * Function callback: Construct form for Stage 02.
 *
 * Stage 02 form allows user to describe and save a additional trait/s found in the spreadsheet submitted in Stage 01.
 *
 * Assuming the file uploaded properly, we have access to an excel file and need to
 * ensure that all traits have been described. If there are any that haven't then
 * we need to ask the the user to define them now.
 *
 * NOTE: User has the option to skip this stage by not checking any of the new traits found and clicking next step.
 */
function rawpheno_upload_form_stage_review(&$form, &$form_state) {
  // Array to hold new headers.
  $new_header = array();

  // The project id number the spreadsheet and column headers are specific to.
  $project_id = $form_state['values']['sel_project'];

  // FIND NEW HEADERS.
  // First step, determine which headers/traits need to be described.
  if (isset($form_state['multistep_values']['fid'])) {
    // Get Drupal file object.
    $file = file_load($form_state['multistep_values']['fid']);

    // Ensure that the file exits and project id is selected.
    // The form will unset the project id upon page refresh, this will catch the condition
    // when no project id is selected, user will be stopped and is requested to retry the process.
    if ($file AND !empty($project_id)) {
      $new_header = rawpheno_indicate_new_headers($file, $project_id);
      $form_state['multistep_values']['new_headers'] = $new_header;
    }
    else {
      drupal_set_message(t('Unable to access your file. Please try uploading again.'), 'error');
    }
  }
  else {
    drupal_set_message(t('We have no record of your uploaded file. Please try uploading it again.'), 'error');
  }

  // If we were unable to access the file or project is not selected then don't let them proceed.
  if (!isset($file) OR empty($file) OR empty($project_id)) {
    $form['notice'] = array(
      '#type' => 'markup',
      '#markup' => '<div class="messages error">'
                   . t('Unable to access uploaded file. Please attempt to upload your file again <a href="@upload-page">on the previous page</a>. If the problem persists then contact the administrator.',
                     array('@upload-page' => url('phenotypes/raw/upload')))
                   . '</div>',
    );

    // No submit button as well.
    unset($form['next_step']);

    return $form;
  }


  // NO NEW HEADER.
  // If there are no new headers then they don't have to do anything. The module will display a summary
  // showing the number of parsed column headers in the spreadsheet.
  if (empty($new_header)) {
    $all_headers = rawpheno_all_headers($file);

    $markup = '
      <ul class="error-main">
        <li class="info">
          <em>' . count($all_headers) . ' Column Headers.</em>
        </li>

        <li class="info">
          <em>No Additional Column Headers Found.</em>
        </li>
      </ul>
    ';

    $form['xls_summary_fldset'] = array(
      '#type' => 'fieldset',
      '#title' => t('Describe new trait'),
    );

    $form['xls_summary_fldset']['information'] = array(
      '#type' => 'markup',
      '#markup' => $markup,
    );

    $form['notice'] = array(
      '#type' => 'markup',
      '#markup' => '<div class="messages status">No new traits were detected in the spreadsheet. Please click "Next Step".</div>',
    );

    return $form;
  }


  // NEW HEADERS FOUND.
  $cv = tripal_get_cv(array('name' => 'phenotype_measurement_types'));
  $cv_id = $cv->cv_id;

  // Where clause that form part of the SQL below.
  $where = array(
    'yes' => "TRIM(LOWER(name)) = :cvterm LIMIT 1",
    'no'  => "TRIM(LOWER(SPLIT_PART(name, '(', 1))) LIKE :cvterm"
  );

  $sel = "SELECT * FROM {cvterm} WHERE
          cvterm_id NOT IN (SELECT cvterm_id FROM pheno_project_cvterm WHERE project_id = :project_id)
          AND cv_id = :cv_id AND %s";

  // Otherwise, we need a form!
  // Main fieldset container for form elements.
  $form['xls_review_fldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Check the traits that you want to describe and save'),
  );

  $headers_no_format = array_map('rawpheno_function_delformat', $new_header);

  // Array to hold all checked headers.
  $arr_checked_header = array();

  foreach($new_header as $i => $k) {
    if (isset($k) AND !empty($k)) {
      // To prevent spills of information to other form set, reset this variable that holds
      // query result object.
      if (isset($cvterm_info)) {
        unset($cvterm_info);
      }

      // CHECKBOX to let user select a trait to describe and save. If left unchecked, system will not save it.
      $form['xls_review_fldset']['chk_' . $i] = array(
        '#type' => 'checkbox',
        '#title' => t(ucwords($k)),
        '#ajax' => array(
          'callback' => 'ajax_rawpheno_upload_form_step2_expand_trait_callback',
          'wrapper' => 'trait-description-' . $i,
          'effect' => 'fade',
          'trait_index' => $i,
        ),
      );

      // Container div that holds form elements.
      $form['xls_review_fldset']['fldset_' . $i] = array(
        '#type' => 'markup',
        '#prefix' => '<div id="trait-description-' . $i . '" class="container-trait">',
        '#suffix' => '</div>',
      );

      // By default, show the describe form.
      $show_form = 'yes';

      // If the checkbox is checked then show the fields user want to described.
      if (isset($form_state['values']['chk_' . $i]) AND ($form_state['values']['chk_' . $i] == TRUE)) {
        // TERM NAME/TRAIT/HEADER
        $form['xls_review_fldset']['fldset_' . $i]['txt_header_' . $i] = array(
          '#type' => 'hidden',
          '#value' => $k,
        );

        // Clean up the current header.
        $name = trim(strtolower(preg_replace('!\s+!', ' ', $k)));
        $arr_checked_header[] = $name;

        // Test if the header has a unit component and set the variable accordingly.
        $has_unit = (strpbrk($name, '()')) ? 'yes' : 'no';

        // Format the name to be used in the following SQL. When the name has a unit component,
        // we just feed the name to the SQL using equal operator, otherwise, we use like operator
        // to find all similar headers and suggest it.
        $cvterm = ($has_unit == 'yes') ? $name : '%' . $name . '%';

        // Construct the query statement.
        $sql = sprintf($sel, $where[$has_unit]);
        $args = array(':project_id' => $project_id, ':cv_id' => $cv_id, ':cvterm' => $cvterm);
        $h = chado_query($sql, $args);

        if ($h->rowCount() > 0) {
          if ($has_unit == 'yes') {
            // Load information about the header.
            $cvterm_info = $h->fetchObject();

            // Tell user that the column header exists already.
            $form['xls_review_fldset']['fldset_' . $i]['notice_' . $i] = array(
              '#markup' => '<div class="form-item">The system has detected this column header in the database.
                            All form fields are disabled to prevent alteration to the original version.
                            To save this header and data associated to it, please keep the checkbox checked.</div>',
            );

            $show_form = 'yes';
          }
          else {
            // Suggest similar header.
            // Ensure that the list of headers to be suggested is not in the list of headers detected,
            // that way we can avoid duplicate headers.
            $header_options = array();
            foreach($h as $m) {
              $this_header = trim(strtolower($m->name));

              if (!in_array($this_header, $headers_no_format)) {
                $header_options[$m->cvterm_id] = $m->name;
              }
            }

            if (count($header_options) > 0) {
              $form['xls_review_fldset']['fldset_' . $i]['sel_header_' . $i] = array(
                '#type' => 'select',
                '#title' => t('Did you mean?'),
                '#options' => array('-1' => '---', 0 => 'None of these apply') + $header_options,
                '#ajax' => array(
                  'callback' => 'ajax_rawpheno_upload_form_step2_load_header_info',
                  'wrapper' => 'trait-description-' . $i,
                  'effect' => 'fade',
                  'trait_index' => $i,
                ),
                '#element_validate' => array('rawpheno_newheader_didyoumean_validate'),
                '#attributes' => array('class' => array('sel-header')),
                '#description' => t('The system has detected a similar header in the database.
                                     It is recommended that you select the header from the select box that best describes your data.
                                     If the header is not listed, please select None of these apply option and use the form below to describe this column header.'),
              );

              $show_form = 'no';
            }
            else {
              $show_form = 'yes';
            }

            if (isset($form_state['values']['sel_header_' . $i])) {
              if ($form_state['values']['sel_header_' . $i] > 0) {
                $cvterm_id = $form_state['values']['sel_header_' . $i];
                $cvterm_info = tripal_get_cvterm(array('cvterm_id' => $cvterm_id));
                $show_form = 'yes';
              }
              elseif ($form_state['values']['sel_header_' . $i] == 0) {
                $show_form = 'yes';
              }
            }
          }
        }


        if ($show_form == 'yes') {
          // TERM DEFINITION
          $form['xls_review_fldset']['fldset_' . $i]['txt_def_' . $i] = array(
            '#type' => 'textarea',
            '#title' => t('Definition'),
            '#required' => TRUE,
            '#description' => t('A human-readable text definition'),
          );

           if (isset($cvterm_info) && isset($cvterm_info->definition)) {
             $form['xls_review_fldset']['fldset_' . $i]['txt_def_' . $i]['#value'] = $cvterm_info->definition;
             $form['xls_review_fldset']['fldset_' . $i]['txt_def_' . $i]['#disabled'] = TRUE;
           }


          // UNIT
          $form['xls_review_fldset']['fldset_' . $i]['txt_unit_' . $i] = array(
            '#type' => 'textfield',
            '#title' => t('Unit'),
            '#required' => TRUE,
            '#maxlength' => 100,
            '#element_validate' => array('rawpheno_newheader_unit_validate'),
            '#description' => t('Unit of measurement used'),
          );

          if (isset($cvterm_info)) {
            $unit_val = strpbrk($cvterm_info->name, '()');
            // Remove any parenthesis making its way to the final value.
            $unit_val = str_replace(array('(', ')'), '', $unit_val);

            $form['xls_review_fldset']['fldset_' . $i]['txt_unit_' . $i]['#value'] = trim($unit_val);
            $form['xls_review_fldset']['fldset_' . $i]['txt_unit_' . $i]['#disabled'] = TRUE;
          }
          else {
            if ($has_unit == 'yes') {
              $form['xls_review_fldset']['fldset_' . $i]['txt_unit_' . $i]['#value'] = trim(strpbrk($name, '()'), '()');
              $form['xls_review_fldset']['fldset_' . $i]['txt_unit_' . $i]['#disabled'] = FALSE;
            }
          }


          // DESCRIPTION - describe the trait.
          $form['xls_review_fldset']['fldset_' . $i]['txtarea_describe_' . $i] = array(
            '#type' => 'textarea',
            '#title' => t('Describe the method used'),
            '#required' => TRUE,
            '#description' => t('Describe the method used to collect this data if you used a scale, be specific'),
          );

          if (isset($cvterm_info)) {
            $cvterm_describe_unit = rawpheno_function_cvterm_properties($cvterm_info->cvterm_id);

            $form['xls_review_fldset']['fldset_' . $i]['txtarea_describe_' . $i]['#value'] = $cvterm_describe_unit;
            $form['xls_review_fldset']['fldset_' . $i]['txtarea_describe_' . $i]['#disabled'] = TRUE;
          }


          // Note fields are required
          $form['xls_review_fldset']['fldset_' . $i]['required_' . $i] = array(
            '#markup' => '<div class="form-item">* means field is required</div>'
          );
        }
      }
    }
  }

  // Hidden field containing all the checked new headers
  if (isset($arr_checked_header) AND count($arr_checked_header) > 0) {
    $form['all_header_checked'] = array(
      '#type' => 'hidden',
      '#value' => implode(',', $arr_checked_header),
    );
  }

  // Indicator to user of how many of the new traits found has been described.
  $form['traits_checked'] = array(
    '#type' => 'markup',
    '#markup' => '<div class="messages status">You have described <span id="traits-checked">0</span> trait. Please click "Next Step".</div>'
  );

  return $form;
}


/**
 * Function validate the unit field when new header is detected in the spreadsheet.
 * Validation includes ensuring that user does not use parenthesis ( and ) in the unit.
 */
function rawpheno_newheader_unit_validate($element, &$form_state) {
  $unit_value = trim($element['#value']);
  $project_id = $form_state['values']['sel_project'];

  // strpbrk() Returns a string starting from the character found, or FALSE if it is not found.
  if (strpbrk($unit_value, '()')) {
    form_set_error($element['#name'], 'The value in the unit field contains characters "(" and/or ")". Please remove these characters and try again.');
  }
  else {
    // Test the name plus the unit combination if it is in the project.
    $header_field = str_replace('unit', 'header', $element['#name']);
    $header_value = $form_state['values'][$header_field];

    if (!strpbrk($header_value, '()')) {
      $name_value = trim(strtolower($header_value . ' (' . strtolower($unit_value) . ')'));

      $sql = "SELECT cvterm_id
              FROM {cvterm} INNER JOIN pheno_project_cvterm USING(cvterm_id)
              WHERE project_id = :project_id AND TRIM(LOWER(name)) = :cvterm LIMIT 1";

      $args = array(':project_id' => $project_id, ':cvterm' => $name_value);
      $h = chado_query($sql, $args);

      if ($h->rowCount() == 1) {
        form_set_error($element['#name'], 'Cannot save column header and unit. ' . ucfirst($name_value) . ' exists in this project.');
      }

      // Test if user is about to save same headers.
      if (isset($form_state['values']['all_header_checked'])) {
        $h = $form_state['values']['all_header_checked'];
        $header_validation = explode(',', $h);

        if (in_array($name_value, $header_validation)) {
          form_set_error($element['#name'], 'Cannot save multiple entries of the same column header and unit combination.');
        }
      }
    }
  }
}


/**
 * Function callback: validate Did you mean? select box
 */
function rawpheno_newheader_didyoumean_validate($element, &$form_state) {
  if ($element['#value'] < 0) {
    form_set_error($element['#name'], 'Please select an option and try again.');
  }
}


/**
 * Function load column header information.
 */
function ajax_rawpheno_upload_form_step2_load_header_info($form, $form_state) {
  $i = $form_state['triggering_element']['#ajax']['trait_index'];

  return $form['xls_review_fldset']['fldset_' . $i];
}


/*
 * Selects the piece of the form we want to use as replacement text and returns it as a form (renderable array).
 *
 * @return renderable array (the trait description elements)
 */
function ajax_rawpheno_upload_form_step2_expand_trait_callback($form, $form_state) {
  // Unique id of each form set.
  $i = $form_state['triggering_element']['#ajax']['trait_index'];

  return $form['xls_review_fldset']['fldset_' . $i];
}


/**
 * Function callback: Construct form for Stage 03.
 *
 * Stage 03 form is the final stage that displays a status message
 * and a navigation button to direct user after a successful file upload.
 */
function rawpheno_upload_form_stage_save($form, &$form_state) {
  $job_id = NULL;
  global $user;

  if (isset($form_state['build_info']['args'][0])) {
    $job_id = $form_state['build_info']['args'][0];

    // We only want to run jobs that has phenotypic data in it. Otherwise we tell user
    // job is not valid (in case user will hack the url containing the job id).
    // Retrieve the tripal job and determine the percent complete.
    $job = tripal_get_job($job_id);

    // If job is valid.
    if ($job) {
      if ($job->uid == $user->uid) {
        // Job id belongs to the user. Authorized.
        $job_status = trim(strtolower($job->status));

        // Check if it is a valid job and not a BLAST or other job type.
        if ($job->callback == 'rawpheno_load_spreadsheet') {
          if ($job_status == 'completed') {
            // Is completed some time ago.
            $form['notice'] = array(
              '#markup' => '<div class="messages error">It appears that you are attempting to submit a spreadsheet that has been processed already</div>'
            );
          }
          elseif ($job_status == 'error') {
            // Has error.
            $form['notice'] = array(
              '#markup' => '<div class="messages error">It appears that you are attempting to submit a spreadsheet that has errors.</div>'
            );
          }
          elseif ($job_status == 'cancelled') {
            // Is cancelled.
            $form['notice'] = array(
              '#markup' => '<div class="messages error">It appears that you are attempting to submit a spreadsheet that has been cancelled.</div>'
            );
          }
          else {
            // Not processed yet - show the progress bar.
            // A valid job - work on it.
            $form['notice'] = array(
              '#type' => 'markup',
              '#markup' =>
                  '<div class="messages status">Your spreadsheet has been successfully submitted and <em>will not be interupted</em> if you choose to leave this page.</div>'
                . '<div class="messages warning">The progress bar below indicates our progress updating ' . strtoupper($_SERVER['SERVER_NAME']) . '. <em>Your data will not be available until the progress bar below completes</em>.</div>'
            );

            // Add Progress JS Library.
            drupal_add_js('misc/progress.js');

            // This is the link passed to the JavaScript Progress.js as the parameter to a function
            // that monitors a link. The link is a function callback that generates a JSON object
            // containing the number of rows save in percent. See file: rawpheno.module.
            $form['tripal_job_id'] = array(
              '#type' => 'hidden',
              '#value' => base_path() . '/phenotypes/raw/upload/job_summary/' . $job->job_id,
              '#attributes' => array('id' => 'tripal-job-id'),
            );

            // We make a DIV which the progress bar can occupy. You can see this in use
            // in ajax_example_progressbar_callback().
            $form['status'] = array(
              '#type' => 'markup',
              '#markup' => '<div class="progress-pane"></div>'
            );
          }
        }
        else {
          // Job not supported by this module.
          $form['notice'] = array(
            '#markup' => '<div class="messages error">It appears that you are attempting to request a process that is not supported by this module.</div>'
          );
        }
      }
      else {
        // Not authorized.
        $form['notice'] = array(
          '#markup' => '<div class="messages error">It appears that you are attempting to submit a spreadsheet that is not in your account.</div>'
        );
      }
    }
    else {
      // Job is not valid or does not exists.
      $form['notice'] = array(
        '#markup' => '<div class="messages status">The job request to save spreadsheet file does not exist.</div>'
      );
    }
  }

  return $form;
}


/**
 * Implements hook_file_insert().
 * Save file information when file is saved (backup).
 *
 * @param $file
 *   Drupal file opbject.
 */
function rawpheno_file_insert($file) {
  // Process file only when there is a request to save a file and that request is coming from backup page.
  if (isset($file->source)) {
    if ($file->source == 'bdnd') {
      // User id of the currently logged in user.
      $user_id = $GLOBALS['user']->uid;
      // The project id field.
      $project_id = $_POST['backup_sel_project'];
      // The notes field.
      $notes = trim(strip_tags($_POST['backup_txt_description']));

      // Query the record id of the project to user record.
      // The result id will be used to map a backup file to user and to project.
      $sql = "SELECT project_user_id FROM pheno_project_user WHERE project_id = :project_id AND uid = :user_id LIMIT 1";
      $args = array(':project_id' => $project_id, ':user_id' => $user_id);
      $prj_usr_id = db_query($sql, $args)
        ->fetchField();

      // Get the validation result performed to the spreadsheet file and store the result along with the file information.
      // The same validation process performed in upload data page is carried out to backup file. However, the result
      // is stored as plain text and passed and failed icons are replaced by words passed and failed, respectively.
      $status = rawpheno_validate_excel_file($file, $project_id, 'backup');

      $s = (isset($status['status'])) ? $status['status'] : $status;

      // Express the validation result array into human readable non-html content format.
      $validation_result = '';
      // Call the same validator function used in upload data.
      $validators = module_invoke_all('rawpheno_validators');

      // For each status result, convert it to text based and add a unique text indicator to be used
      // as key to explode the entire text and create a list using the <br> tag.
      foreach($s as $key => $result) {
        $flag = ($result === TRUE) ? 'passed' : 'failed';

        // The item keyword will be used to create a list of validation entries when displaying validaiton result to user.
        $validation_result .= '#item: (' . $flag . ') ' . $validators[$key]['label'] . "\n";
        if ($result !== TRUE) {
          $message = call_user_func($validators[$key]['message callback'], $result);
          if (!empty($message)) {
            $validation_result .= implode("\n", $message);
          }
        }
      }

      // Compute the version number of this file.
      $sql = "SELECT MAX(t2.version) + 1 AS version
              FROM {pheno_project_user} AS t1 RIGHT JOIN {pheno_backup_file} AS t2 USING(project_user_id)
              WHERE t1.project_id = :project_id AND t1.uid = :user_id LIMIT 1";

      $args = array(':project_id' => $project_id, ':user_id' => $user_id);
      $version = db_query($sql, $args)
        ->fetchField();

      // On initial upload the file version is null, in this case
      // version is set to 1. Version is incremented by 1 (+1) in
      // subsequent uploads.
      $version = ($version === null) ? 1 : $version;

      // Insert a record of this file.
      // File version, which is a sequential order (integer) is handled by the rdbms as it is set to serial type.
      if (isset($status['check_limit'])) {
        $validation_result .= "#item: (failed) NOTICE : " . $status['check_limit'] . "\n";
      }

      db_insert('pheno_backup_file')
        ->fields(array('fid' => $file->fid,
                       'notes' => $notes,
                       'version' => $version,
                       'project_user_id' => $prj_usr_id,
                       'validation_result' => $validation_result))
        ->execute();

      // Finally, make the file permanent.
      rawpheno_upload_make_file_permanent($file->fid);

      // Make the validation result available to frontend.
      $_SESSION['rawpheno']['backup_file_validation_result'] = $status;
    }
  }
}


/**
 * Upload validators callback:
 * Basic compliance test to spreadsheet submitted.
 *
 * @param $file
 *   Drupal file opbject.
 */
function rawpheno_file_validate($file) {
  // 10 mins (60 * 10).
  $max_time = 600;

  if ($file->source == 'dnd') {
    // Set processing time to 10 mins.
    ini_set('max_execution_time', $max_time);

    // Upload data page.

    // Project id number the spreadsheet and column headers are specific to.
    $project_id = (int)$_POST['sel_project'];

    // Validate the file.
    // The following function will return an array specifying which of the validation
    // steps passed and providing infomration for those that failed.
    $status = rawpheno_validate_excel_file($file, $project_id, 'upload');

    // We want to show the user which steps passed/failed even if all of them passed,
    // so lets do that now. We use drupal_set_message() because returning from this function
    // creates an error message and halts file upload, whereas, using drupal_set_message()
    // allows us to print to the screen regardless of failure/success.
    drupal_set_message(theme('rawpheno_upload_validation_report', array('status' => $status)), 'rawpheno-validate-progress');

    // Now we want to determine if validation passed or failed as a whole.
    // To do that we have to look at each step and only if all steps passed
    // did the file pass validation and can be uploaded.
    $all_passed = TRUE;
    foreach ($status as $test_result) {
      if ($test_result !== TRUE) {
        $all_passed = FALSE;
        break; break;
      }
    }

    // hook_file_validate() expects an array of error messages if validation failed and
    // and empty array if there are no errors. We don't want this system to print the errors
    // for us since we are using our more friendly theme (see drupal_set_message() above).
    // The work-around is to pass FALSE if validation failed.
    if ($all_passed) {
      drupal_set_message('<span id="rawpheno-upload-successful">Your file uploaded successfully. Please click "Next" to continue.</span>');
      return array();
    }
    else {
      return FALSE;
    }
  }
  elseif ($file->source == 'bdnd') {
    // Set processing time to 10 mins.
    ini_set('max_execution_time', $max_time);

    // Backup file page.

    // Array to hold the validation result.
    $status = array();

    // Project id number the spreadsheet and column headers are specific to.
    $project_id = (int)$_POST['backup_sel_project'];

    // Perform basic compliance test:
    // - A project is selected.
    // - File is Microsoft Excel Spreadhseet file.
    // - Measurement tab exists.
    // - Essential column headers defined in the project are present.
    $flag_index = array('project_selected', 'is_excel', 'tab_exists', 'column_exists');

    // Validate file.
    $flags = rawpheno_validate_excel_file($file, $project_id, 'backup');

    $s = (isset($flags['status'])) ? $flags['status'] : $flags;

    // Read only the status from test listed above.
    foreach($s as $i => $v) {
      if (in_array($i, $flag_index) AND ($v === FALSE || $v === 'todo')) {
        $status[$i] = $v;
      }
    }

    // When any of the mentioned test failed, show them to user.
    if (count($status) > 0) {
      if (isset($flags['check_limit'])) {
        drupal_set_message($flags['check_limit'], 'error');
      }

      drupal_set_message(theme('rawpheno_upload_validation_report', array('status' => $status)), 'rawpheno-validate-progress');
      return FALSE;
    }

    // Else, proceed to hook_file_insert().
  }
  else {
    // File source is one that is not of interest to us.
    // Do not return anything or it will trigger validation errors for other modules.
  }
}


/**
 * Make the phenotype excel file permanent on successful upload.
 *
 * This is an additional process handler used by the form API to generate the form array
 * for a given element. Usually it is used to make a custom form element or enhance a
 * standard form element.
 *
 * We are using it to capture the just uploaded file within the AJAX call by checking
 * when the element is rendered if it has a fid (ie: has been saved).
 */
function rawpheno_phenotype_upload_file_element_process($element, &$form_state, $form) {
  if (isset($element['#value']['fid']) AND !empty($element['#value']['fid'])) {
    $file_id = $element['#value']['fid'];
    rawpheno_upload_make_file_permanent($file_id);
  }

  return $element;
}


/**
 * Make the file uploaded permanent and make a record indicating that file is used by the module.
 *
 * @param $file_id
 *   File id in Drupal file object.
 */
function rawpheno_upload_make_file_permanent($file_id) {
  // Get the file object.
  $file = file_load($file_id);

  if ($file) {
    // Make the file permanent.
    $file->status = FILE_STATUS_PERMANENT;
    file_save($file);

    // Also, point out that we are using it ;-)
    // Note, the file_usage_add() function expects a numerical unique id which we don't have.
    // We have gotten around this by using the uid concatenated with the timestamp using
    // the assumption that a single user cannot upload more than one phenotype file within a second.
    file_usage_add($file, 'rawpheno', 'rawphenotypes-file', $file->uid . $file->timestamp);
  }
}


/**
 * Implements hook_validate().
 */
function rawpheno_upload_form_master_validate($form, &$form_state) { }


/**
 * Implements hook_submit().
 *
 * Master submit to handle form submit.
 */
function rawpheno_upload_form_master_submit(&$form, &$form_state) {
  // Which button triggers a submit action.
  $btn_submit = $form_state['triggering_element']['#value'];

  // Save any additional traits and then submit a job to save the spreadsheet.
  if ($form_state['stage'] == 'review') {
    $job_id = rawpheno_submit_review($form, $form_state);

    // Then we need to add the job_id to the path so the system can keep track of it.
    if ($job_id) {
      drupal_goto(current_path() . '/' . $job_id);
    }
  }

  // If we just uploaded the file then we want to save the fid for easy access.
  if (isset($form_state['values']['dnd'])) {
    $form_state['multistep_values']['fid'] = $form_state['values']['dnd'];
  }

  // If the next step button was pressed then iterate to the next step.
  if ($btn_submit == 'Next Step') {
    // Definitely save the form id.
    if(isset($form_state['multistep_values']['form_build_id'])) {
      $form_state['values']['form_build_id'] = $form_state['multistep_values']['form_build_id'];
    }

    // Save the values from the current step.
    $form_state['multistep_values'][$form_state['stage']] = $form_state['values'];

    // Iterate to the next step.
    $form_state['new_stage'] = rawpheno_next_page($form, $form_state);

    // Ensure the form state is saved and the form is rebuilt.
    $form_state['multistep_values']['form_build_id'] = $form_state['values']['form_build_id'];
    $form_state['stage'] = $form_state['new_stage'];
    $form_state['rebuild'] = TRUE;
  }
}


/**
 * Save spreadsheet to database.
 */
function rawpheno_submit_review($form, &$form_state) {
  // Project id number the spreadsheet and column headers are specific to.
  $project_id = $form_state['values']['sel_project'];

  // Save spreadsheet data in the following order.
  // 1. New column headers.
  // 2. The entire spreadsheet.

  // cvterm id of controlled vocabulary.
  $cvid = tripal_get_cv(array('name' => 'phenotype_measurement_units'));
  $cv_measurements_unit = $cvid->cv_id;

  // 1. Save new headers.
  // Read variable that holds new column headers.
  $new_header = $form_state['multistep_values']['new_headers'];

  // Create an array of new hearders with flag/status if user wants to save it.
  // This array will be passed to rawpheno_load_spreadsheet.
  $arr_newheaders = array();

  // Determine if there is new header.
  if (count($new_header) > 0) {
    $trait_type = rawpheno_function_trait_types();

    // In some cases, the unit contains 1st; 2nd; followed by the actual unit.
    // Attempt to extract known unit.
    // Known units:
    // date, count, days, cm, scale, g, text, y/n/?;
    $def_unit = rawpheno_function_default_unit('def');
    $unit_set = array_keys($def_unit);

    // Count trait that has been checked for saving.
    $has_newtraits_checked = 0;

    // Read each column header.
    foreach($new_header as $i => $header) {
      // For each new header store information provided in the interface.
      // Indicates if user has check this header for saving.
      $header = trim(str_replace(array("\n", "\r", "  "), ' ', $header));
      $header = preg_replace('/\s+/', ' ', $header);

      $arr_newheaders[$header]['flag'] = ($form_state['values']['chk_' . $i] == 1) ? 1 : 0;

      // Determine if the form in review traits has been filled out and checkbox
      // has been checked by user. If it has been checked then save the trait.
      if ($form_state['values']['chk_' . $i] === 1 && !empty($form_state['values']['txt_header_' . $i])) {
        $has_newtraits_checked++;

        // Examine the unit and find any default unit.
        // This will be use to test this new trait if data match the unit.
        $tmp_unit = $form_state['values']['txt_unit_' . $i];

        foreach($unit_set as $def_u) {
          if (strpos($tmp_unit, $def_u) !== FALSE) {
            $tmp_unit = $def_u;

            break;
          }
        }

        // If none detected, send the entire value/unit to validator
        // which will evaluate to text.

        $arr_newheaders[$header]['unit'] = $tmp_unit;

        // Before save, we need to tell if the header is present in the database and
        // user just wants to reuse them. Otherwise, add a new header.
        // Reuse header - set to OPTIONAL.
        if ((isset($form_state['values']['sel_header_' . $i]) AND $form_state['values']['sel_header_' . $i] > 0) OR
            (isset($form_state['values']['txt_header_cvterm_id_' . $i]))) {

          // User selected from a list of similar headers.
          $cvterm_id = (isset($form_state['values']['sel_header_' . $i]))
            ? $form_state['values']['sel_header_' . $i]
            : $form_state['values']['txt_header_cvterm_id_' . $i];

          // Map this header to the project.
          $sql = "SELECT cvterm_id FROM {pheno_project_cvterm} WHERE project_id = :project_id AND cvterm_id = :cvterm_id LIMIT 1";
          $args = array(':project_id' => $project_id, ':cvterm_id' => $cvterm_id);

          $h = db_query($sql, $args);
          if ($h->rowCount() <= 0) {
            // Add to project only when it is not in the project.
            // Set the trait type to contributed.
            db_insert('pheno_project_cvterm')
              ->fields(array(
                'project_id' => $project_id,
                'cvterm_id' => $cvterm_id,
                'type' => $trait_type['type2'])
              )
              ->execute();
          }

          // When saving this data for this header, use the cvterm_id.
          $arr_newheaders[$header]['alt_header'] = $cvterm_id;
          continue;
        }

        // Check if the trait exists in the database, then it is likely
        // that the user is reusing the trait - threfore it is not contributed and just map
        // the cvterm id to a project.

        // Add the as contributed - set to CONTRIBUTED.
        // Construct the column header name.
        $name   = trim($form_state['values']['txt_header_' . $i]);
        $name   = preg_replace('/\s+/', ' ', $name);

        $unit   = trim($form_state['values']['txt_unit_' . $i]);
        $method = trim($form_state['values']['txtarea_describe_' . $i]);
        $def    = trim($form_state['values']['txt_def_' . $i]);

        // Format the header.
        if (strpbrk($name, '()')) {
          // Header has a unit part.
          $name = trim(str_replace(array("\n", "\r", "  "), ' ', $name));
        }
        else {
          // Construct header plus the unit.
          $name = $name . ' (' . strtolower($unit) . ')';
        }

        // Trait properties - use when inserting the cterm and reference to other property.
        $m_cvterm = array(
          'id'         => 'rawpheno_tripal:' . $name,
          'name'       => $name,
          'definition' => $def,
          'cv_name'    => 'phenotype_measurement_types'
        );

        // Search the name in cvterm and decide if trait should be considered optional or contributed.
        $sql = "SELECT t2.cvterm_id
                FROM {cv} AS t1 INNER JOIN {cvterm} AS t2 USING (cv_id)
                WHERE
                  trim(lower(t2.name)) = trim(lower(:cvterm_name))
                  AND t1.name = :cv_name LIMIT 1";

        $args = array(':cvterm_name' => $m_cvterm['name'], ':cv_name' => $m_cvterm['cv_name']);
        $result = chado_query($sql, $args)
          ->fetchObject();

        if ($result) {
          // Found use the id.
          $m_cvterm_id = $result->cvterm_id;
          // Trait is optional.
          $type = $trait_type['type2'];
        }
        else {
          // Not found, insert and get the inserted id.
          $m = tripal_insert_cvterm($m_cvterm);
          $m_cvterm_id = $m->cvterm_id;
          // Trait is contributed.
          $type = $trait_type['type5'];
        }

        // When saving this data for this header, use the cvterm_id.
        $arr_newheaders[$header]['alt_header'] = $m_cvterm_id;
        db_insert('pheno_project_cvterm')
          ->fields(array('project_id' => $project_id,
                         'cvterm_id' => $m_cvterm_id,
                         'type' => $type))
          ->execute();

        // Create a R Friendly version.
        $r_version = rawpheno_function_make_r_compatible($m_cvterm['name']);
        $cv_rfriendly = tripal_get_cv(array('name' => 'phenotype_r_compatible_version'));

        $values = array(
          'cvterm_id' => $m_cvterm_id,
          'type_id'   => $cv_rfriendly->cv_id,
          'value'     => $r_version,
          'rank'      => 0
        );

        chado_insert_record('cvtermprop', $values);

        // Then save the Unit.
        $u_cvterm = array(
          'id'         => 'rawpheno_tripal:' . strtolower($unit),
          'name'       => strtolower($unit),
          'definition' => $unit,
          'cv_name'    => 'phenotype_measurement_units'
        );

        $u_cvterm_id = chado_select_record('cvterm',array('cvterm_id'),
                                           array('name' => $u_cvterm['name'],
                                           'cv_id' => array('name' => $u_cvterm['cv_name'])));
        if (!$u_cvterm_id) {
          $u_cvterm_id = tripal_insert_cvterm($u_cvterm);
        }

        // Grab just the id.
        if (is_array($u_cvterm_id)) {
          $u_cvterm_id = $u_cvterm_id[0]->cvterm_id;
        }
        elseif (is_object($u_cvterm_id)) {
          $u_cvterm_id = $u_cvterm_id->cvterm_id;
        }

        // Don't forget the method description.
        $prop = array(
          'cvterm_id' => $m_cvterm_id,
          'type_id'   => $cv_measurements_unit,
          'value'     => $method,
          'rank'      => 0,
        );

        $prop_id = chado_select_record('cvtermprop', array('cvtermprop_id'), $prop);
        if (!$prop_id) {
          $prop = chado_insert_record('cvtermprop', $prop);
        }

        // Finally relate the measurement and unit.
        $rel = array(
          'subject_id' => $u_cvterm_id,
          'type_id'    => $cv_measurements_unit,
          'object_id'  => $m_cvterm_id,
        );
        $rel_id = chado_select_record('cvterm_relationship', array('cvterm_relationship_id'), $rel);
        if (!$rel_id) {
          chado_insert_record('cvterm_relationship', $rel);
        }
      }
    }
  }

  // 2. The entire spreadsheet and validate new traits..
  // Get the variable that holds the path to the spreadsheet file in the server.
  $file = file_load($form_state['multistep_values']['fid']);
  $xls_file = drupal_realpath($file->uri);

  // Before saving, double check to ensure that new traits user wants in the system
  // must have data that matches the unit is measuring. When a trait fails validation,
  // we flag it as such then it will be ignored in the save process.
  // Additional email notification to admin (email support) about this trait.

  // Find all traits checked out by user.

  // Validate all checked traits.
  if ($has_newtraits_checked > 0) {
    // Get headers and cell values in rows and columns used for validation.
    // Number of rows to check.
    $rows_to_check = 20;

    // Add the libraries needed to parse excel files.
    rawpheno_add_parsing_libraries();
    $xls_obj = rawpheno_open_file($file);

    // Change to the correct spreadsheet.
    rawpheno_change_sheet($xls_obj, 'measurements');

    $n_i = 0;

    foreach ($xls_obj as $rows) {
      // Headers.
      if ($n_i == 0) {
        foreach($rows as $row_index => $row_header) {
          $h = trim(str_replace(array("\n", "\r", "  "), ' ', $row_header));
          $h = preg_replace('/\s+/', ' ', $h);

          if (in_array($h, array_keys($arr_newheaders)) && $arr_newheaders[$h]['flag'] == 1) {
            // Save the index number.
            $arr_newheaders[$h]['file_index'] = $row_index;
          }
        }

        $n_i++;
        continue;
      }

      // Cell/Data.
      foreach($rows as $cell_index => $cell_data) {
        foreach($arr_newheaders as $nt_header => $nt_prop) {
          if ($nt_prop['file_index'] == $cell_index && $nt_prop['flag'] == 1) {
            $cell_data = trim($cell_data);

            $arr_newheaders[$nt_header]['test'][]
              = rawpheno_function_match_unit($cell_data, $nt_prop['unit'], $nt_header);
          }
        }
      }

      // Stop at # of rows.
      if ($n_i == $rows_to_check) {
        break;
      }

      $n_i++;
    }
  }


  // Array of required traits excluding Name.
  $plantprop_headers = rawpheno_project_plantproperty_traits($project_id);

  // Drupal user object.
  global $user;

  if (isset($xls_file) && !empty($xls_file)) {
    $job_id = tripal_add_job(
      "Upload Phenoypic data: " . $xls_file,
      'rawpheno',
      'rawpheno_load_spreadsheet',
      array(
        $project_id,
        serialize($arr_newheaders),
        $form_state['multistep_values']['fid'],
        serialize($plantprop_headers),
      ),
      $user->uid
    );

    return $job_id;
  }
}
