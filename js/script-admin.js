/**
 * @file
 * Manage behavior in project management admin control panel
 */
 (function($) {
  Drupal.behaviors.adminProjectManagement = {
    attach: function (context, settings) {
      var headers = 'container-prj-hdr';
      var users = 'container-prj-usr';
      var envdata = 'container-prj-env';

      $('#nav-tabs li').click(function(i) {

        if ($(this).index() == 0) {
          // Column headers tab.
          $('#' + envdata).hide();
          $('#' + users).hide();
          $('#' + headers).show();
        }
        else if ($(this).index() == 1) {
          // Active users tab.
          $('#' + envdata).hide();
          $('#' + headers).hide();
          $('#' + users).show();
        }
        else {
          $('#' + users).hide();
          $('#' + headers).hide();
          $('#' + envdata).show();
        }

        $('li.active-tab').removeClass('active-tab');
        $(this).addClass('active-tab');
      });

      // Confirm if user wants to proceed with the command.
      $('.link-del').click(function(event) {
        var r = confirm("Are you sure you want to delete?");
        if (!r)  return false;
      });


      // Reveal validation result text.
      $('div.container-cell').click(function() {
        // Examine the height. When height is 50px, user wants to disclose the entire cell contents
        // else, restore it to initial state.
        var id = $(this).attr('id');
        var i = id.replace(/vn-file-|vr-file-/, '');

        var h = $(this).css('height');
        if (h == '65px') {
          $('#vr-file-' + i).css('height', '100%');

          if ($('#vn-file-' + i)) {
            $('#vn-file-' + i).css('height', '100%');
          }
        }
        else {
          $('#vr-file-' + i).css('height', '65px');

          if ($('#vn-file-' + i)) {
            $('#vn-file-' + i).css('height', '65px');
          }
        }
      });


      // Add event listener to user my folder.
      $('.link-show-folder').click(function(e) {
        e.preventDefault();

        var lnk = $(this);
        var attrId = lnk.attr('id');
        var myFolder = '#show-' + attrId;

        if (lnk.text() == '[Show]') {
          lnk.text('[Hide]');
          $(myFolder).show();

          // Reset any view open.
          var ls = $('.link-show-folder');
          $.each(ls, function(key, value) {
            var c = $(this);
            var i = c.attr('id');
            if (i != attrId) {
              c.text('[Show]');
              $('#show-' + i).hide();
            }
          });

        }
        else {
          lnk.text('[Show]');
          $(myFolder).hide();
        }
      });
    }
  };
}(jQuery));
