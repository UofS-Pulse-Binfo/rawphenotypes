<?php
/**
 * @file
 * Master template file of rawpheno.
 *
 * Available variables:
 * - $path: The directory path to rawpheno module.
 * - $hostname: Base host name.
 * - $page_id: String ID of the page/form.
 * - $rel_url: Each page has a link to a related page.
 * - $theme_colour: Colour setting selected by user in administration panel. Default to navyblue/#304356.
 * - $page_title: Page title from admin configuration.
 * - $page_url: An array containing url of pages.
 */
?>

<div id="rawpheno-notification-need-help" title="Need Help?">
  <div><a href="raw/videos">Need Help?</a></div>
</div>

<?php
 $class = 'rawpheno-notification-item rawpheno-notification-title rawpheno-' . $alert;
 $hi_user = 'Hi ' . $username . '!';
?>

<div class="<?php print $class; ?>" title="<?php print $hi_user; ?>">
  <div class="rawpheno-item-wrapper">
    Raw Phenotypes
    <h3><?php print $hi_user; ?></h3>
  </div>
</div>

<div class="rawpheno-notification-hline">&nbsp;</div>

<div id="rawpheno-notification-dashboard">
  <div class="rawpheno-notification-item rawpheno-item-a" title="Raw Phenotypes Backup Spreadsheet File">
    <div class="rawpheno-item-wrapper">
      <h3>You have <span class="ap-em-count"><?php print $file_count; ?> Files</span> in your Backups</h3>
      <a href="<?php print $path_phenotypes_raw . 'backup'; ?>" class="rawpheno-notification-arrow-right">Backup Spreadsheet File</a> |
      <a href="#" id="rawpheno-notification-why-backup-link">Why Backup?</a>
    </div>

    <div id="rawpheno-notification-info-why-backup">
      <div id="rawpheno-notification-arrow-head">&nbsp;</div>
      <div id="rawpheno-notification-info-copy">
        <p>
          <img src="<?php print $path_module . 'theme/img/cloud-image.png'; ?>" id="rawpheno-notification-image" />
          <h3>You can never have too many Backups!</h3>
          It is important to backup your data collection spreadsheets on <?php print $hostname; ?>
          servers to ensure your supervisor and/or colleagues have access to the data. Furthermore, it protects the data
          in case of tablet/computer failure, forgotten passwords, corrupted files, incomplete file transfers, sudden rainstorms, etc. Remember, you can never have too many backups!
        </p>
        <a href="#" id="rawpheno-notification-ok-button">Ok, got it!</a>
      </div>
    </div>
  </div>

  <div class="rawpheno-notification-item rawpheno-item-b" title="Raw Phenotypes Upload Data">
    <div class="rawpheno-item-wrapper">
      <h3>Upload data to <?php print $hostname; ?></h3>
      <a href="<?php print $path_phenotypes_raw . 'upload'; ?>" class="rawpheno-notification-arrow-right">Upload Data</a> |
      <a href="<?php print $path_phenotypes_raw . 'instructions'; ?>">Download Standard Spreadsheet File</a>
    </div>
  </div>
</div>

<div id="rawpheno-notification-help-topics">
  <div class="rawpheno-notification-item rawpheno-item-c" title="Raw Phenotypes Help Topics">
    <div class="rawpheno-item-wrapper">
      <h3>Need Help? Select a topic below.</h3>
      <ul>
        <li><a href="<?php print $path_phenotypes_video . 'rawpheno_backup.mp4'; ?>" target="_blank" class="rawpheno-notification-arrow-right">How to Backup Data Collection Spreadsheet File</a></li>
        <li><a href="<?php print $path_phenotypes_video . 'rawpheno_upload.mp4'; ?>" target="_blank" class="rawpheno-notification-arrow-right">How to Upload Data to <?php print strtoupper($_SERVER['SERVER_NAME']); ?></a></li>
        <li><a href="<?php print $path_phenotypes_raw   . 'instructions'; ?>" target="_blank" class="rawpheno-notification-arrow-right">Standard Phenotyping Procedure</a></li>
        <li><a href="<?php print $path_phenotypes_raw   . 'videos'; ?>" target="_blank" class="rawpheno-notification-arrow-right">Other Video Demonstrations</a></li>
      </ul>
    </div>
  </div>
</div>
