/**
 * Stage 03 - Save Spreadsheet.
 * Create progress bar showing number of records save in percent.
 */
(function ($) {
  Drupal.behaviors.addProgressBar = {
    attach: function (context, settings) {
      // Link to monitor. The link contains the job id and is
      // accessing a page callback that generates a JSON object of
      // the percent completed.
      var job_id = $('#tripal-job-id').val();

      // Initializes the progress bar.
      // This will terminate by itself when progress is null.
      pb = new Drupal.progressBar('trpdownloadProgressBar', function(percentage, message) {
        // Also ensure that we stop when the progress bar is complete ;-).
        if (percentage == '100') {
          pb.stopMonitoring();

          // Remove the target = _blank  from the list of links below the progress bar.
          $('.nav-buttons').attr('target', '');
        }
      });

      // Adds the progress bar to the div we created above.
      // Start the progress bar at 0 - Waiting instead of 1%.
      pb.setProgress(0, '');
      $('.progress-pane').append(pb.element);

      pb.startMonitoring(job_id, 500);
    }
  };
}(jQuery));
