/**
 * @file
 * Allow user to collapse upload instructions message box and other windows.
 */
(function($) {
  Drupal.behaviors.rawphenoToggleWindows = {
    attach: function (context, settings) {
      // Allow user to collapse upload information window.
      $('hr.button-collapse-infowindow').click(function() {
        var selected = $(this);
        var divContainer = selected.prev('div.window-info');
        if ($(this).hasClass('window-on')) {
          divContainer.stop().animate({height : '0'}, 300, function() {
            selected.removeClass('window-on');
          });
        }
        else {
          // Get the height of div.
          var h = divContainer.css('height', '100%').height();
          divContainer.css('height', 0);
          
          // Use the height to animate.
          divContainer.stop().animate({height : h}, 300, function() {
            selected.addClass('window-on');
          });
        }
      }); 
    }
  };
}(jQuery));