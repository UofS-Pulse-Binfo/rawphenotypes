<?php
/**
 * @file
 * Master template file of rawpheno.
 *
 * Available variables:
 * - $path: The directory path to rawpheno module.
 * - $page_id: String ID of the page/form.
 * - $rel_url: Each page has a link to a related page.
 * - $theme_colour: Colour setting selected by user in administration panel. Default to navyblue/#304356.
 * - $page_title: Page title from admin configuration.
 * - $page_url: An array containing url of pages.
 */


 // Template structure:
 // <div>title</div>       - Window title
 // <div>                  - Main div
 //   <div>                - div sub title
 //     <div>left</div>    - Left column div  - page subtitle, form fields, state indicator
 //     </div>right</div>  - Right column div - navigation button to related page
 //   </div>
 //
 //   <div>content</div>   - Content div
 // </div>


 // Only the upload page has this page id. Other pages, for instance,
 // Rawdata page has page id rawpheno_rawdata and Download page has page id
 // rawpheno_download and so on. For consistency, when page id is rawpheno_upload_form_master
 // which is the upload data page, replace it with rawpheno_upload.
 $page_id = ($form['#form_id'] == 'rawpheno_upload_form_master')
   ? 'rawpheno_upload' : $form['#form_id'];

 // Content of the secondary page title.
 $subtitle = '';
 if ($page_id == 'rawpheno_instructions') {
   // Search box in instructions page.
   $subtitle .= drupal_render($form['txt_search']);
   $subtitle .= drupal_render($form['btn_search']);
 }
 elseif ($page_id == 'rawpheno_upload') {
   // Stage indicator in upload page.
   $current_stage = $form['current_stage']['#value'];
   $stages = array_keys($upload_stages);
   $stage_number = array_search($current_stage, $stages) + 1;
   $subtitle .= 'Stage ' . $stage_number . ' of 3 - ' . $upload_stages[$current_stage];
 }
 elseif ($page_id == 'rawpheno_backup') {
   // Currently logged in user. Show the name of the user.
   $subtitle .= $GLOBALS['user']->name;
 }
 else {
   // Default to space to prevent the container from collapsing.
   $subtitle .= '&nbsp;';
 }


 // Markup the button that links to related page.
 //  - Upload page and Instructions page.
 //  - Instructions page and Upload data page.
 //  - Rawdata page and Download data page.
 //  - Download page and Rawdata page.
 //  - Backup page and Instructions page.
 $form['page_button']['#prefix'] = '<a href="' . $rel_url . '" style="background-color: '.$theme_colour.'">';
 $form['page_button']['#suffix'] = '</a>';

 if ($page_id == 'rawpheno_rawdata') {
   $ver = rawpheno_function_d3_version();
   $ver = explode('.', $ver);
 }
?>


<div class="container-header" style="<?php print "background-color: $theme_colour;"; ?>">
  <?php print $page_title; ?>
</div>


<div class="container-page" style="<?php print "border-color: $theme_colour;" ?>">
  <div class="content-wrapper">
    <?php
    if (preg_match('/(?i)msie [1-8]/', $_SERVER['HTTP_USER_AGENT'])) {
      // Browser not supported.
    ?>
      <div id="container-no-info" class="messages warning">
        Browser not supported. Please update your browser.
      </div>
    <?php
      unset($form);
    }
    elseif (!module_exists('dragndrop_upload_element') AND $page_id == 'rawpheno_upload') {
      // Test if page is upload page and Drag and Drop module is present and enabled.
    ?>
      <div id="container-no-info" class="messages warning">
        This module requires Drag and Drop Upload module to be installed and enabled. Please contact the administrator of this website.
      </div>
    <?php
      // Tell administrator to download and install Drag and Drop module.
      $link_to_git_rawphenotypes = l('UofS Pulse Binfo - Rawphenotypes', 'https://github.com/UofS-Pulse-Binfo/rawphenotypes');
      print tripal_set_message('Administrators, check to ensure that Drag and Drop module is installed and enabled in this site. To review external module dependencies of this module click the link below: <br />' . $link_to_git_rawphenotypes, TRIPAL_INFO, array('return_html' => TRUE));
      unset($form);
    }
    elseif (!rawpheno_function_project()) {
      // No project available.
    ?>
      <div id="container-no-info" class="messages warning">
        There is no project available in this module. Please contact the administrator of this website.
      </div>
    <?php
      // Admin: Create and configure projects:
      $link_to_manage_project = l('Rawphenotypes: Manage Project', '/admin/tripal/extension/rawphenotypes');
      print tripal_set_message('Administrators, you can create or configure Phenotyping Projects by clicking the link below: <br />' . $link_to_manage_project, TRIPAL_INFO, array('return_html' => TRUE));
      unset($form);
    }
    elseif (!rawpheno_function_data() AND in_array($page_id, array('rawpheno_rawdata', 'rawpheno_download'))) {
      // Has project but project has no data associated to it.
      // Excempt pages upload, backup and instructions.
    ?>
      <div id="container-no-info" class="messages warning">
        There is no project with data available in this module.
      </div>
    <?php
      // Admin: Create and configure projects:
      $link_to_manage_project = l('Rawphenotypes: Manage Project', '/admin/tripal/extension/rawphenotypes');
      print tripal_set_message('Administrators, you can create or configure Phenotyping Projects by clicking the link below: <br />' . $link_to_manage_project, TRIPAL_INFO, array('return_html' => TRUE));
      unset($form);
    }
    elseif (count($my_prj = rawpheno_function_user_project($GLOBALS['user']->uid)) < 1) {
      // User is not appointed to project.
    ?>
      <div id="container-no-info" class="messages warning">
        You have no projects assigned to your account. Please contact the administrator of this website.
      </div>
    <?php
      // Admin: Appoint user to upload data to project.
      $link_to_manage_project = l('Rawphenotypes: Manage Project', '/admin/tripal/extension/rawphenotypes');
      print tripal_set_message('Administrators, you can appoint users to upload data to Phenotyping Projects by clicking the link below: <br />' . $link_to_manage_project, TRIPAL_INFO, array('return_html' => TRUE));
      unset($form);
    }
    elseif (file_prepare_directory($pub_dir = 'public://') == FALSE AND in_array($page_id, array('rawpheno_upload', 'rawpheno_backup'))) {
      // Upload destination directory is not writable.
    ?>
      <div id="container-no-info" class="messages warning">
        The file destination directory is not writable. Please contact the administrator of this website.
      </div>
    <?php
      unset($form);
    }
    elseif ($page_id == 'rawpheno_rawdata' AND ($ver[0] != '3' OR !libraries_load('d3js'))) {
    ?>
      <div id="container-no-info" class="messages warning">
        Failed to initiliaze visualization library. Please contact the administrator of this website.
      </div>
    <?php
      $link_to_manage_project = l('D3 - Data Driven Documents', 'https://github.com/d3/d3/releases/download/v3.5.14/d3.zip');
      print tripal_set_message('Administrators, it appears that the site is using D3 version not supported by this module. Please download a D3 version 3.5.14 by clicking on the link below: <br />' . $link_to_manage_project, TRIPAL_INFO, array('return_html' => TRUE));
      unset($form);
    }
    else {
      // Project is available.
    ?>
      <div class="container-subtitle">
        <div class="subtitle-left"><?php print $subtitle; ?></div>
        <div class="subtitle-right"><?php print drupal_render($form['page_button']); ?></div>
      </div>

      <div class="container-contents">
      <?php
      if ($page_id == 'rawpheno_rawdata') {
      // BEGIN rawdata page.
      ?>
        <div id="container-marker-information" title="Click to clear chart">
          <h2 id="title-pheno">&nbsp;</h2>
          &nbsp;: are measured in <em id="text-rep">&nbsp;</em> with a leaf symbol (<span>&nbsp;</span>) <a href="#">Clear chart</a>
        </div>

        <div id="container-form-select">
          <div class="sel-projects"><?php print drupal_render($form['rawdata_sel_project']); ?></div>

          <div class="sel-traits">
          <?php
            $project_ids = explode(',', $form['rawdata_txt_project']['#value']);
            foreach($project_ids as $p) {
              print drupal_render($form['sel_' . $p]);
            }
          ?>
          </div>
        </div>

        <div id="container-rawdata" class="form-wrapper clear-float"><?php print drupal_render_children($form); ?></div>
      <?php
      // END rawdata page.
      }


      elseif ($page_id == 'rawpheno_download') {
      // BEGIN download page.
      ?>
        <div id="container-download" class="form-wrapper"><?php print drupal_render_children($form); ?></div>
      <?php
      } // END download page.


      elseif ($page_id == 'rawpheno_upload') {
      // BEGIN upload page.
      ?>
        <div id="container-upload">
          <div id="container-progress">
            <a href="#" id="link-help">Need help?</a>

            <div id="container-help-information" title="Click to collapse Help Window">
              <ul>
                <li>
                  <h2>Stage 1 - Validate Spreadsheet</h2>
                  This form will guide you through uploading your raw phenotypic data. Your data should be in a
                  <em>Microsoft Excel Workbook (XLSX) following the format described on the
                  <a href="<?php print $page_url['rawpheno_instructions'] ?>" target="_blank">Instructions Page</a></em>.
                </li>

                <li>
                  <h2>Stage 2 - Describe New Trait</h2>
                  In the second step we ask that you <em>describe any additional phenotypes</em>.
                </li>

                <li>
                  <h2>Stage 3 - Save Spreadsheet</h2>
                  Finally, the spreadsheet is saved to <?php print strtoupper($_SERVER['SERVER_NAME']); ?>, at which point the
                  <em>phenotypic data is available through
                  <a href="<?php print $page_url['rawpheno_rawdata'] ?>" target="_blank">summaries</a> and
                  <a href="<?php print $page_url['rawpheno_download'] ?>" target="_blank">downloads</a></em>.
                </li>
              </ul>
            </div>

            <div>
              <?php
                print drupal_render($form['header_upload']);
              ?>
              <div class="clear-float"></div>
            </div>
          </div>

          <?php
          // In Stage #3 Save Spreadsheet, show progress bar and links to other pages
          if ($form['current_stage']['#value'] == 'save') {
            print drupal_render($form['sel_project']);
          ?>
            <fieldset>
              <legend><span class="fieldset-legend">Spreadsheet submitted</span></legend>
              <div class="fieldset-wrapper">
                <?php  print drupal_render($form['notice']); ?>
                <div class="container-status"><?php print drupal_render($form['status']); ?></div>
              </div>
            </fieldset>

            <div class="container-buttons">
              <div class="buttons-wrapper">
                <a href="<?php print $page_url['rawpheno_upload']; ?>" target="_blank" class="nav-buttons"><span>Upload New Data</span></a>
                <a href="<?php print $page_url['rawpheno_download']; ?>" target="_blank" class="nav-buttons"><span>Download Data</span></a>
                <a href="<?php print $page_url['rawpheno_rawdata']; ?>" target="_blank" class="nav-buttons"><span>Data Summary</span></a>
                <a href="<?php print $page_url['rawpheno_instructions']; ?>" target="_blank" class="nav-buttons"><span>Standard Procedure</span></a>
                <div class="clear-float"></div>
              </div>
            </div>

          <?php
            }

            print drupal_render_children($form);
          ?>
          </div>

        <?php
        } // End rawdata page.


        elseif ($page_id == 'rawpheno_backup') {
        // BEGIN backup page.
        ?>
          <div id="container-backup">
            <?php
              // No project is associated to the user account.
              if (isset($form['no_data'])) {
                print drupal_render($form['no_data']);
              }
            ?>

            <div id="container-add-file">
              <?php
                // When a file is uploaded.
                print drupal_render($form['message_upload_result']);
                print drupal_render($form['validation_result']);
                print drupal_render($form['link_upload_file']);

                // Upload file interface.
                print drupal_render($form['backup_sel_project']);
                print drupal_render($form['bdnd']);
                print drupal_render($form['backup_txt_description']);
                print drupal_render($form['backup_file_submit']);

                print drupal_render($form['summary']);
              ?>
            </div>

            <div>
              <h2>Manage My Files:</h2>
            </div>

            <?php
              print drupal_render($form['tbl_root_dir']);
              print drupal_render_children($form);
            ?>
          </div>
        <?php
        } // END upload page.


        elseif ($page_id == 'rawpheno_instructions') {
        // BEGIN instructions page.
          // Display error message when project id is not valid.
          print drupal_render($form['message_invalid_project']);
        ?>
          <div id="container-project-panel">
            <?php print drupal_render($form['project_panel']); ?>
            <span><a href="#">Change Project</a></span>
          </div>

          <div id="container-sel-project">
             <?php print drupal_render($form['sel_project']); ?>
          </div>

          <div id="container-instructions">
            <div id="phenotype-page">
              <div id="container-search-result"></div>

              <div id="tabs">
                <ul>
                  <li><a href="#fragment-1">Standard Procedure</a></li>
                  <li id="essential"><a href="#fragment-2">Essential Traits</a></li>

                  <?php
                    // Test if optional traits have traits defined.
                    $trait_type = rawpheno_function_trait_types();
                    if ($form['tbl_project_headers_' . $trait_type['type2']]['#markup'] != 'no-trait') {
                  ?>
                  <li><a href="#fragment-3">Optional Traits</a></li>
                  <?php } ?>

                  <?php
                    // Test if subset traits have traits defined.
                    if ($form['tbl_project_headers_' . $trait_type['type3']]['#markup'] != 'no-trait') {
                  ?>
                  <li><a href="#fragment-4">Subset Traits</a></li>
                  <?php } ?>

                  <li id="photo-appendix"><a href="#fragment-5">Photo Appendix</a></li>
                  <li id="reference"><a href="#fragment-6">Reference</a></li>
                </ul>

                <div id="fragment-1">
                  <h3>Spreadsheet Tips</h3>
                  <ul>
                    <li><p>Comments with instructions, tips and scales are included with the header for each trait. Simply click the speech bubble icon beside the header.</p></li>
                    <li><p>Essential Traits have green headers; all other traits are optional.</p></li>
                    <li><p>You should hide optional traits you are not taking data for by long pressing a column header then selecting “hide”.</p></li>
                    <li><p>We’ve included an easy calculator to determine “Days from Planting” in a separate tab.</p></li>
                    <li><p>Special Interest Traits: <br />Add a column on the spreadsheet for any other trait you are interested in collecting data for. When uploading you will be asked to provide a description including units or scale used to take the measurement.</p></li>
                  </ul>

                  <div id="container-resource-links">
                    <div>
                      <h3>&raquo; To Collect Data:</h3>
                      <p><?php print drupal_render($form['download_data_collection']); ?></p>
                    </div>

                    <div>
                      <h3>&raquo; To Backup File:</h3>
                      <p><?php print l('Backup Data Collection Spreadsheet', './phenotypes/raw/backup') ?></p>
                    </div>

                    <div>
                      <h3>&raquo; To Submit Data:</h3>
                      <p><?php print l('Upload Phenotypic Data Page', './phenotypes/raw/upload') ?></p>
                    </div>
                  </div>

                  <div class="clear-float">&nbsp;</div>
                </div>

                <?php
                $arr_type_note = array(
                  2 => 'These traits are essential to this project and data should be collected for all genotypes sent to you.',
                  3 => 'These traits are optional. We will be taking them in our location and have thus provided our procedure in case you interested in taking these data in your location as well. Feel free to record ANY data you are interested in (including traits not listed below –just add a column to the accompanying data spreadsheet for traits not listed below).',
                  4 => 'The following traits require a fair amount of work and, as such, are completely optional. We will collect them in SK, if you are interested in these traits, please contact us to make sure we are collecting the same thing. Note: These columns are hidden by default. If you would like to record this data, select columns “X” and "AB" and either right-click (computer) or long-press (tablet) the column header then select “Unhide”. For the following “Subset Traits”, select 2 plants from the middle of each plot and randomly collect 10 peduncles from each plant, ranging from the top to bottom, for a total of 20 peduncles. If 20 peduncles cannot be obtained, sample from a 3rd plant.',
                  5 => 'These traits are optional. Please contact the Project Manager if you are interested in these traits.'
                );

                $i = 2;

                // Plant property traits not included.
                unset($trait_type['type4']);

                foreach($trait_type as $type) {
                  if ($form['tbl_project_headers_' . $type]['#markup'] == 'no-trait') {
                    unset($form['tbl_project_headers_' . $type]);
                  }
                  else {
                    // Notes:
                    if (trim($form['project_panel']['#markup']) == 'AGILE: Application of Genomic Innovation in the Lentil Economy') {
                      $notes = $arr_type_note[$i];
                    }
                    else {
                      $notes = $arr_type_note[5];
                    }

                    print '<div id="fragment-' . $i . '">';
                    print '<h3>' . $notes . '</h3>';
                    print drupal_render($form['tbl_project_headers_' . $type]);
                    print '</div>';
                  }

                  $i++;
                }
                ?>

                <div id="fragment-5">
                  <h3>Topic: <select>
                    <option value="0">Tendrils</option>
                    <option value="1">Pods</option>
                  </select></h3>

                  <div id="photo-container">
                    <div id="gallery-container">
                      <div class="side-nav"><a href="javascript:void();"><</a></div>
                      <div class="gallery-img">
                        <input type="hidden" id="path" value="<?php print base_path() . $path; ?>/theme/img/appendix/">
                        <input type="hidden" id="cur-img" value="0">
                        <img src="<?php print base_path() . $path; ?>/theme/img/appendix/01-tendrils-no-elongation.jpg">
                        <br /><em>No elongation</em>
                      </div>
                      <div class="side-nav"><a href="javascript:void();">></a></div>
                      <div style="clear:both">&nbsp;</div>
                    </div>
                  </div>
                </div>

                <div id="fragment-6">
                  <h3>Further reference for Reproductive stages:</h3>
                  <em>Erskine et al. (1990) Stages of Development in Lentil. Experimental Agriculture. 26(3): 297-302.</em>
                  <ul>
                    <li><strong>R1 - First Bloom</strong><p>One open flower at any node</p></li>
                    <li><strong>R3 - Early Pod</strong><p>Pod on nodes 10-13 of the basal primary branch visible</p></li>
                    <li><strong>R5 - Full Seed</strong><p>Seeds in any single pod on nodes 10-13 of the basal primary branch are swollen and completely fill the pod cavity</p></li>
                    <li><strong>R7 - Physiological Maturity</strong><p>The leaves start yellowing and 50% of the pods have turned yellow</p></li>
                  </ul>
                </div>
              </div>
            </div>
          </div>
        <?php
          print drupal_render_children($form);
        } // END instructions page.
        ?>

      </div>

    <?php
    }
    ?>
  </div>
</div>
