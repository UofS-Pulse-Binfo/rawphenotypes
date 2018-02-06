/**
 * @file
 * Manage behavior in notification block.
 */
(function($) {
  Drupal.behaviors.notificationBlock = {
    attach: function (context, settings) {
      // Reference elements.
      var helpLink = $('#rawpheno-notification-need-help a');
      var helpTopic = $('#rawpheno-notification-help-topics');
      var myDashboard = $('#rawpheno-notification-dashboard');
      var whyBackupLink = $('#rawpheno-notification-why-backup-link');
      var whyBackupInfo = $('#rawpheno-notification-info-why-backup');
      var okBackup = $('#rawpheno-notification-ok-button');

      // Need Help?
      helpLink.click(function(e) {
        e.preventDefault();

        var btn = $(this);
        if (btn.text() == 'Close Help') {
          btn.text('Need Help?');

          helpTopic.slideUp().hide()
          myDashboard.slideDown();
        }
        else {
          btn.text('Close Help');

          myDashboard.slideUp().hide();
          helpTopic.slideDown()
        }
      });

      // Why Backup?
      whyBackupLink.click(function(e) {
        e.preventDefault();

        whyBackupInfo.show().slideDown();
      });

      // Why Backup acknowledged.
      okBackup.click(function(e) {
        e.preventDefault();

        whyBackupInfo.slideUp().hide();
      });

      // Convert help topic select box to a jump menu.
      if ($('#rawpheno-notification-helptopic-select').length) {
        $('#rawpheno-notification-helptopic-select').change(function() {
          var url = $(this).attr('value');

          if (url) {
            window.open(url, '_blank');
          }
        });
      }

    }
  };
}(jQuery));
