<?php
/**
 * @file
 * Contains backup interface of this module.
 */

/**
 * Function callback: construct backup interface.
 */
function rawpheno_backup($form, &$form_state, $project_id = NULL, $command = 'desc', $file_id_no = NULL) {
  // Attach CSS.
  $path = drupal_get_path('module', 'rawpheno') . '/theme/';
  $form['#attached']['css'] = array($path . 'css/rawpheno.backup.style.css');

  // Projects that have associated column header set and must have at least 1 essential column header.
  // The projects are filtered to show only projects assigned to user.
  $all_project = rawpheno_function_user_project($GLOBALS['user']->uid);

  // No projects assined to the user.
  if (count($all_project) < 1) {
    return $form;
  }

  // Navigation button. Related page of rawdata/summary page is download page.
  $form['page_button'] = array(
    '#type' => 'markup',
    '#markup' => t('Standard Procedure &#10095;'),
  );


  // After a backup file, user is redirected to raw/backup/up. This means that user has backed up a file
  // and that in hook_file() function a session is set to contain the validation result of the validation
  // performed to the file. This block is to show that validation result to user.
  if ($project_id == 'up' AND isset($_SESSION['rawpheno']['backup_file_validation_result'])) {
    // Read the session variable.
    $status = $_SESSION['rawpheno']['backup_file_validation_result'];

    // Tell user that a file has been successfully backed up.
    $form['message_upload_result'] = array(
      '#markup' => '<div class="messages status">You have successfully uploaded a file into your account.
                    Below is the result of the validation performed to the file.</div>',
    );

    // Uset the validation result theme used in upload page to theme the validation result.
    if (isset($status['status'])) {
      $check_limit_message = '<div class="messages warning">' . $status['check_limit'] . '</div>';
      $status = $status['status'];
    }
    else {
      $check_limit_message = '';
    }

    $form['validation_result'] = array(
      '#markup' => $check_limit_message . theme('rawpheno_upload_validation_report', array('status' => $status)),
    );

    // Finally, add a link to upload another file.
    $form['link_upload_file'] = array(
      '#markup' =>  l(t('Backup Another File'), './phenotypes/raw/backup',
                      array('attributes'=> array('id' => 'link-success-upload-file')))
    );

    // A session variable is set in hook_insert_file() that contains the validation result of the
    // previous file backup. When user wants to upload another file, reset this session variable.
    unset($_SESSION['rawpheno']['backup_file_validation_result']);
  }


  // Archive and delete file commands.
  if (isset($file_id_no) AND $file_id_no > 0) {
    // Define valid commands.
    $arr_valid_commands = array('archive', 'restore', 'delete', 'desc', 'asc');

    // Test if file exists before modifying the file.
    $sql = "SELECT t2.file_id, t2.fid
            FROM {pheno_project_user} AS t1 INNER JOIN {pheno_backup_file} AS t2 USING(project_user_id)
            WHERE t2.file_id = :file_id AND t1.project_id = :project_id AND t1.uid = :user_id
            LIMIT 1";

    $args = array(':file_id' => $file_id_no, ':project_id' => $project_id, ':user_id' => $GLOBALS['user']->uid);
    $f = db_query($sql, $args);

    if ($f->rowCount() == 1 AND in_array($command, $arr_valid_commands)) {
      if ($command == 'archive') {
        // Archive the file.
        db_update('pheno_backup_file')
          ->fields(array('archive' => 'y'))
          ->condition('file_id', $file_id_no, '=')
          ->execute();
      }
      elseif ($command == 'restore') {
        // Restore the file.
        db_update('pheno_backup_file')
          ->fields(array('archive' => 'n'))
          ->condition('file_id', $file_id_no, '=')
          ->execute();
      }
      elseif ($command == 'delete') {
        // Physically delete file.
        // Delete record in pheno_backup_file.
        $num_deleted = db_delete('pheno_backup_file')
          ->condition('file_id', $file_id_no, '=')
          ->condition('archive', 'y', '=')
          ->execute();

        // To delete a file in Drupal use file_delete function.
        // Get file object and load it to file delete funciton.
        $xlsfile = $f->fetchObject();

        $file = file_load($xlsfile->fid);
        file_delete($file, TRUE);
      }
    }
    else {
      // File not found;
      drupal_set_message(t('Command cannot be executed to this file.'), 'error');
    }
  }


  // FORM.
  // Construct backup file form.
  // Array to hold project assigned to user.
  $arr_my_project = $all_project;

  if (!isset($project_id) AND $project_id <= 0) {
    if (count($arr_my_project) > 0) {
      // Default to the only project when there is a single project assigned to user
      // otherwise, let user select a project.
      if (count($arr_my_project) > 1) {
        // When there is more that 1 project, tell user to select a project.
        $project_options = array(0 => '---') + $arr_my_project;
      }
      else {
        // Else, default to the only project available.
        $project_options = $arr_my_project;
      }

      // Yes, user has at least a project in the account.
      $form['backup_sel_project'] = array(
        '#type' => 'select',
        '#title' => t('Please select a project:'),
        '#options' => $project_options,
        '#default_value' => array_keys($project_options)[0],
        '#id' => 'backup_sel_project'
      );

      // Create an instance of DragNDrop Upload.
      // SETTINGS:
      //  #file_upload_max_size: max file size allowed
      //  #upload_location: destination of file
      //  #upload_event: manual - show an upload button or auto - uploads after drag drop
      //  #upload_validators: allowed file extensions
      //  #upload_button_text: label of upload button
      //  #droppable_area_text: text in drop area
      //  #progress_indicator: none, throbber or bar
      //  #progress_message: message to display while processing
      //  #allow_replace: allow user to replace file by drag and drop another file
      //  #standard_upload: show browse button or not
      //  #upload_button_text: submit button text (not required when auto submit is auto)

      $form['bdnd'] = array(
        '#type' => 'dragndrop_upload',
        '#title' => t('Spreadsheet file:'),
        '#file_upload_max_size' => '10M',
        '#upload_location' => 'public://',
        '#upload_event' => 'manual',
        '#upload_validators' => array('file_validate_extensions' => array('xlsx xls')),
        '#droppable_area_text' => t('Drag your Microsoft Excel Spreadsheet file here'),
        '#progress_indicator' => 'throbber',
        '#progress_message' => 'Please wait...',
        '#allow_replace' => 1,
        '#standard_upload' => 1,
        '#id' => 'backup_dropzone',
        '#upload_button_text' => t('Upload'),
      );

      $form['backup_txt_description'] = array(
        '#type' => 'textarea',
        '#title' => t('Describe spreadsheet file:'),
        '#description' => t('Use this field to add comments, description or notes to the spreadsheet file that you want to backup'),
        '#rows' => 2,
        '#resizable' => FALSE,
      );
    }
    else {
      // User has no project in the account.
      $form['no_data'] = array(
        '#markup' => t('You have no project in your account. Please contact the administrator of this website.'),
        '#prefix' => '<div id="container-no-info" class="messages warning">',
        '#suffix' => '</div>',
      );
    }
  }
  else {
    // User is managing files in a project.
    // Upload is disabled. Display a summary,
    $go_back_link = l('Go back to projects table', 'phenotypes/raw/backup');

    if ($project_id > 0 AND isset($arr_my_project[$project_id])) {
      $form['summary'] = array(
        '#markup' => $go_back_link . t('<h3>Project: @project</h3>', array('@project' => $arr_my_project[$project_id])),
      );
    }
  }


  // Array to hold table headers.
  $arr_headers = array();
  // Array to hold table rows.
  $arr_rows = array();
  // Array to hold archive files.
  $arr_row_archive = array();
  // Array to hold table properties.
  $arr_tbl_args = array(
    'sticky' => TRUE,
  );


  // Create a projects table and projects file table (file browser). If a project_id is supplied
  // create a table listing all files in that project, otherwise, the default, create a table
  // listing all projects the user is assigned to and a summary of the number of backed up file
  // a project has.

  // Create a table showing files backed up in a project.
  if (isset($project_id) AND $project_id > 0) {
    // Test that sort request is valid. If not in the valid sort array default to desc.
    $sort = (in_array(trim($command), array('asc', 'desc'))) ? $command : 'desc';

    // Query files in the project.
    $sql = "SELECT
              t2.file_id, t2.version, t2.validation_result, t2.notes,
              t3.filename, t3.timestamp, t3.filesize, t3.uri, t2.archive
            FROM
              {pheno_project_user} AS t1
              INNER JOIN {pheno_backup_file} AS t2 using(project_user_id)
              INNER JOIN {file_managed} AS t3 using(fid)
            WHERE t1.project_id = :project_id AND t1.uid = :user_id
            ORDER BY t3.timestamp %s";
    $sql = sprintf($sql, strtoupper($sort));

    $args = array(':project_id' => $project_id, ':user_id' => $GLOBALS['user']->uid);
    $my_files = db_query($sql, $args);

    // Create table elements.
    // Rows.
    if ($my_files->rowCount() > 0) {
      foreach($my_files as $f) {
        // File header will contain filename, timestamp and filesize.
        $row_file = l($f->filename . '(' . format_size($f->filesize) . ')',
                      file_create_url($f->uri),
                      array('attributes' => array('target' => '_blank')));

        $row_file .= t('<br /><small>Uploaded: @timestamp</small>', array('@timestamp' => format_date($f->timestamp)));

        // Notes.
        $row_notes = (empty($f->notes))
          ? '&nbsp;'
          : '<div class="container-cell" title="Click to expand or collapse" id="vn-file-' . $f->file_id . '">' . $f->notes . '</div>';

        // Validation result.
        $row_validation_entries = str_replace(array('#item: (passed)', '#item: (failed)'),
                                              array('<p class="pass"> (PASSED)', '<p class="fail"> * (FAILED)'),
                                              $f->validation_result) . '</p>';
        $alert = '';
        if (($n = substr_count($row_validation_entries, 'FAILED')) > 0) {
          $alert = '<span class="alert-error" title="File has failed validation notices...">' . $n . ' Validation errors</span>';
        }

        $row_validation = $alert . '<div class="container-cell" id="vr-file-' . $f->file_id . '" title="Click to expand or collapse">'. $row_validation_entries .'</div>';

        // Determine if file is archived or not.
        if ($f->archive == 'n') {
          // File is active.
          $row_download = l('', file_create_url($f->uri),
                            array('attributes' => array('target' => '_blank', 'class' => 'link-download', 'title' => 'Download file')));

          $row_del = l('', './phenotypes/raw/backup/' . $project_id .'/archive/' . $f->file_id,
                       array('attributes' => array('class' => 'link-archive', 'title' => 'Archive file')));

          $arr_rows[] = array(
            $row_file,         // Filename.
            '#' . $f->version, // File version.
            $row_notes,        // Notes, comments to file.
            $row_validation,   // Validation result performed to file.
            $row_download,     // Link to download file.
            $row_del           // Link delete (archive) file.
          );
        }
        else {
          // File is archived.
          $row_download = l('', './phenotypes/raw/backup/' . $project_id .'/restore/' . $f->file_id,
                            array('attributes' => array('class' => 'link-restore', 'title' => 'Restore file')));

          $row_del = l('', './phenotypes/raw/backup/' . $project_id .'/delete/' . $f->file_id,
                       array('attributes' => array('class' => 'link-delete', 'title' => 'Delete file')));

          $arr_row_archive[] = array(
            $row_file,         // Filename.
            '#' . $f->version, // File version.
            $row_notes,        // Notes, comments to file.
            $row_validation,   // Validation result performed to file.
            $row_download,     // Link to download file.
            $row_del           // Link to delete (physically delete) file.
          );
        }
      }
    }
    else {
      // No files in a project.
      $arr_tbl_args['empty'] = t('No files in this project.');
    }

    // Header.
    $s = ($sort == 'desc') ? 'asc' : 'desc';
    // Make the varsion column header an active link to sort records by version number.
    $header_sort = l('Version', 'phenotypes/raw/backup/' . $project_id . '/' . $s);
    array_push($arr_headers, t('File'), t($header_sort), t('Notes'), t('Validation Result'), '-', '-');

    // Table id.
    $attr_table_id = 'tbl-user-project-file';
  }
  else {
    // By default, show a table showing summary of projects assigned and the number
    // of files or backed up files in a project.
    $sql = "SELECT COUNT(t2.file_id) AS filecount, t1.project_id
            FROM {pheno_project_user} AS t1 LEFT JOIN {pheno_backup_file} AS t2 USING(project_user_id)
            WHERE t1.uid = :uid AND t1.project_id IN (:my_project_id)
            GROUP BY project_user_id, t1.project_id
            ORDER BY t1.project_id DESC";

    $args = array(':uid' => $GLOBALS['user']->uid, ':my_project_id' => array_keys($arr_my_project));
    $my_files = db_query($sql, $args);

    // Create table elements.
    // Rows.
    if ($my_files->rowCount() > 0) {
      foreach($my_files as $f) {
        $row_view = l('View', './phenotypes/raw/backup/' . $f->project_id);
        $row_prj_name = l($arr_my_project[$f->project_id], 'phenotypes/raw/backup/' . $f->project_id);

        $arr_rows[] = array(
          $row_prj_name,
          $f->filecount,
          $row_view
        );
      }
    }
    else {
      // No project in the account.
      $arr_tbl_args['empty'] = t('No project in your account.');
    }

    // Header.
    array_push($arr_headers, t('Project'), t('File'), t('View'));

    // Table id.
    $attr_table_id = 'tbl-user-project';
  }


  // Update table properties.
  $arr_tbl_args['header'] = $arr_headers;
  $arr_tbl_args['rows'] = $arr_rows;

  // Update table attributes.
  $arr_tbl_args['attributes']['id'] = $attr_table_id;

  // Render the table element.
  $form['tbl_root_dir'] = array(
    '#markup' => theme('table', $arr_tbl_args),
  );


  // When there are archived files, render it as well.
  if (count($arr_row_archive) > 0) {
    // Show archive files table.
    $archive_link = l(count($arr_row_archive). ' Files', '#', array('attributes' => array('class' => 'link-archive')));
    $form['tbl_archive_file'] = array(
      '#type' => 'markup',
      '#markup' => '<div id="container-archive-files">' . $archive_link . '</div>',
    );

    $arr_tbl_args['header'] = $arr_headers;
    $arr_tbl_args['rows'] = $arr_row_archive;
    $arr_tbl_args['attributes']['id'] = 'tbl_project_archive_file';

    $form['tbl_archive_dir'] = array(
      '#markup' => theme('table', $arr_tbl_args),
    );
  }

  // Attach JavaScript.
  $form['#attached']['js']  = array($path . 'js/rawpheno.backup.script.js');

  return $form;
}
