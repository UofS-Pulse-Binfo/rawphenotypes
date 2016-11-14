/**
 * @file
 * Manage behavior in project management admin control panel
 */
(function($) {
  Drupal.behaviors.adminProjectManagement = {
    attach: function (context, settings) {
      var headers = 'container-prj-hdr';
      var users = 'container-prj-usr';

      $('#nav-tabs li').click(function(i) {

        if ($(this).index() == 0) {
          // Column headers tab.
          $('#' + users).hide();
          $('#' + headers).show();
        }
        else {
          // Active users tab.
          $('#' + headers).hide();
          $('#' + users).show();
        }

        $('li.active-tab').removeClass('active-tab');
        $(this).addClass('active-tab');
      });

      // Confirm if user wants to proceed with the command.
      $('.link-del').click(function(event) {
        var r = confirm("Are you sure you want to delete?");
        if (!r)  return false;
      });

    }
  };
}(jQuery));
