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

  // Navigation button. Related page of rawdata/summary page is download page.
  $form['page_button'] = array(
    '#type' => 'markup',
    '#markup' => t('Standard Procedure &#10095;'),
  );
  
  // When project is up, means a file has been upload, then show the validation result to user.
  if ($project_id == 'up') {
    $status = $_SESSION['rawpheno']['backup_file_validation_result'];
    
    $form['message_upload_result'] = array(
      '#markup' => '<div class="messages status">You have successfully uploaded a file in your account. 
                    Below is the result of the validation performed to the file.</div>',
    );
    
    $m = theme('rawpheno_upload_validation_report', array('status' => $status));
    $form['validation_result'] = array(
      '#markup' => $m,
    );
    
    $form['link_upload_file'] = array(
      '#markup' => '<a id="link-success-upload-file" href="../backup">Backup Another File</a>', 
    );
  }
  elseif ($project_id > 0) {
    // A session variable is set in hook_insert_file() that contains the validation result of the
    // previous file backup. When user wants to upload another file, reset this session variable.
    $form['message_upload_result'] = array();
    $form['validation_result'] = array();
    unset($_SESSION['rawpheno']['backup_file_validation_result']);
  }
 
  // Define valid commands.
  $arr_valid_commands = array('archive', 'restore', 'delete', 'desc', 'asc');
  
  // Test if users have assigned project(s) in their account.
  // When there is no project, instruct user to create one and
  // only then they can start uploading files for backup.
  $user_id = $GLOBALS['user']->uid;
  
  // Archive and delete file commands. When command is archive and delete, perform the following
  // block and the rest of the code. Command will be evaluated in later in the code as sorting key.
  if (isset($file_id_no) AND $file_id_no > 0) {
    // Test if file exists before modifying the file.
    $sql = "SELECT t2.file_id, t2.fid 
            FROM {pheno_project_user} AS t1 INNER JOIN {pheno_backup_file} AS t2 USING(project_user_id)
            WHERE t2.file_id = :file_id AND t1.project_id = :project_id AND t1.uid = :user_id
            LIMIT 1";
    
    $args = array(':file_id' => $file_id_no, ':project_id' => $project_id, ':user_id' => $user_id);
    $f = db_query($sql, $args);
    
    if ($f->rowCount() == 1 AND in_array($command, $arr_valid_commands)) {
      if ($command == 'archive') {
        // Archive the file.
        db_update('pheno_backup_file')
          ->fields(array('archive' => 'y'))
          ->condition('file_id', $file_id_no, '=')
          ->execute();  
      }
      elseif($command == 'restore') {
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
        file_delete($file, $force = TRUE);
      }
    }
    else {
      // File not found;
      drupal_set_message(t('Command cannot be executed to this file.'), 'error');
    }
  }
  
  // FORM.
  // Construct backup file form.
  // Query project assigned to user.
  $sql = "SELECT DISTINCT t1.project_id, t2.name 
          FROM pheno_project_user AS t1 LEFT JOIN {project} AS t2 USING(project_id)
          WHERE t1.uid = :uid
          ORDER BY t1.project_id DESC";

  $args = array(':uid' => $user_id);
  $p = chado_query($sql, $args);
  
  // Array to hold project assigned to user.
  $arr_my_project = array();
  
  foreach($p as $m) {
    $arr_my_project[$m->project_id] = $m->name;
  }

  if (!isset($project_id) AND $project_id <= 0) { 
    if (count($arr_my_project) > 0) {
      $my_project = $p->fetchAllKeyed();
      
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
        '#progress_indicator' => '',
        '#progress_message' => '',
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
      // Nope, tell user to add a project before any backup is possible.
      // Display user project file table below to allow user to add project to account.
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
    
    if ($project_id > 0) {
      $form['summary'] = array(
        '#markup' => $go_back_link . t('<h3>Project: @project</h3>', array('@project' => $arr_my_project[$project_id])),
      );
    }  
  }
  
  // List Projects and backed up files.
  // Array to hold table headers.
  $arr_headers = array();
  
  // Array to hold table rows.
  $arr_rows = array();
  
  // Array to hold archive files.
  $arr_row_archive = array();
  
  // Array to hold table properties.
  $arr_tbl_args = array(
    'header' => '',
    'rows' => '',
    'sticky' => TRUE,
    'empty' => 'No files in this project.',
    'attributes' => array('id' => ''),
  );
  
  if (isset($project_id) AND $project_id > 0) {
    // When project id is set, create table showing all files present in a project.
    // Array of valid sort order.
    $arr_valid_sort = array('asc', 'desc');  
    // Test that sort request is valid. If in the valid sort array default to desc.
    $sort = (in_array(trim($command), $arr_valid_sort)) ? $command : 'desc';
    
    // Query files in the project.
    $sql = sprintf("
      SELECT 
        t2.file_id, t2.version, t2.validation_result, t2.notes, 
        t3.filename, t3.timestamp, t3.filesize, t3.uri, t2.archive 
      FROM 
        {pheno_project_user} AS t1
        INNER JOIN {pheno_backup_file} AS t2 using(project_user_id) 
        INNER JOIN {file_managed} AS t3 using(fid)
      WHERE t1.project_id = :project_id AND t1.uid = :user_id
      ORDER BY t3.timestamp %s", strtoupper($sort)
    );
    
    $args = array(':project_id' => $project_id, ':user_id' => $user_id);
    $my_files = db_query($sql, $args);

    // Create table elements.
    // Rows.
    if ($my_files->rowCount() > 0) {
      $files = $my_files->fetchAll();
      foreach($files as $f) {
        // File header will contain filename, timestamp and filesize.
        $row_file = l($f->filename . '(' . format_size($f->filesize) . ')', file_create_url($f->uri), array('attributes' => array('target' => '_blank')));
        $row_file .= t('<br /><small>Uploaded: @timestamp</small>', array('@timestamp' => format_date($f->timestamp))); 
    
        // Validation result.
        $row_validation_entries = str_replace(array('#item: (passed)', '#item: (failed)'), array('<p class="pass"> (PASSED)', '<p class="fail"> * (FAILED)'), $f->validation_result) . '</p>';
        $row_validation = '<div class="container-file-validation-result" id="vr-file-' . $f->file_id . '" title="Click to Show or Hide Validation Result">'. $row_validation_entries .'</div>';
    
        if ($f->archive == 'n') {
          $row_download = l('', file_create_url($f->uri), array('attributes' => array('target' => '_blank', 'class' => 'link-download', 'title' => 'Download file')));
        
          $link_del = url('../../phenotypes/raw/backup/' . $project_id .'/archive/' . $f->file_id); 
          $row_del = l('', $link_del, array('attributes' => array('class' => 'link-archive', 'title' => 'Archive file')));
        
          array_push($arr_rows, array($row_file, '#' . $f->version, $f->notes, $row_validation, $row_download, $row_del));
        }
        else {
          $link_restore = url('../../phenotypes/raw/backup/' . $project_id .'/restore/' . $f->file_id); 
          $row_download = l('', $link_restore, array('attributes' => array('class' => 'link-restore', 'title' => 'Restore file')));
        
          $link_del = url('../../phenotypes/raw/backup/' . $project_id .'/delete/' . $f->file_id); 
          $row_del = l('', $link_del, array('attributes' => array('class' => 'link-delete', 'title' => 'Delete file')));
          
          array_push($arr_row_archive, array($row_file, '#' . $f->version, $f->notes, $row_validation, $row_download, $row_del));
        }
      }
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
            WHERE t1.uid = :uid
            GROUP BY project_user_id, t1.project_id
            ORDER BY t1.project_id DESC";
    
    $my_files = db_query($sql, $args);
    
    // Create table elements.
    // Rows.
    if ($my_files->rowCount() > 0) {  
      $files = $my_files->fetchAll();
      foreach($files as $f) {
        $row_view = l('View', 'phenotypes/raw/backup/' . $f->project_id);
        $row_prj_name = l($arr_my_project[$f->project_id], 'phenotypes/raw/backup/' . $f->project_id);
        array_push($arr_rows, array($row_prj_name, $f->filecount, $row_view)); 
      }
    }
    else {
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
  
  // Archives files table.
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


/**
 * Implements hook_validate().
 * Ensure that project is selected when adding project to account.
 */
function rawpheno_backup_validate($form, &$form_state) {
  $btn_submit = $form_state['triggering_element']['#value'];  
  
  if ($btn_submit == 'Add Project') {
    if($form_state['values']['add_project_select'] == 0) {
      form_set_error('add_project_select', t('No project is selected'));
    }
  }
}