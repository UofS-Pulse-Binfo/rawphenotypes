/**
 * @file
 * Allow user to collapse upload instructions message box and other windows.
 * Notify user to click next step button when does not response to a stage in
 * upload for a specific length of time.
 */
(function($) {
  Drupal.behaviors.rawphenoPageElementBehaviors = {
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
      
      // If after 10 seconds and no response from user,
      // button will start to change color and blink.
      var nextButton = $('#edit-next-step');
      // Fieldset container of describe additional trait.
      // If this fieldset is in DOM, don't apply button animation.
      var fieldSet = ($('#edit-xls-review-fldset').length) ? 1 : 0;

      if (nextButton.length > 0 && nextButton.is(':visible') && fieldSet == 0) { 
        var i = 0;
        var timer = setInterval(function() {
          if (i > 10) { 
            var bg = (i%2 == 0) ? '#F7F7F7' : '#DEDEDE';
            nextButton.css({background: bg});
          }
          i++;
        }, 1000);
      }
    }
  };
}(jQuery));