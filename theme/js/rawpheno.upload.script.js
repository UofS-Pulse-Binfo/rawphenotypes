/**
 * @file
 * Make the drag and drop element toggle it's message on mouseover.
 */
(function($) {
  Drupal.behaviors.rawphenoToggleHelpText = {
    attach: function (contex, settings) {
       $(document).ready(function() { 
        // Allow user to collapse upload instructions message box.
        $('hr').click(function() {
          if ($(this).attr('class') == 'icon-up') {
            // Help window text open.
            $('#txt-help-text').animate({height : '140px'}, 300, function() { 
              $('hr').removeClass('icon-up');
            });
          } 
          else {
            // Help window text collapse.
            $('#txt-help-text').animate({height : '0px'}, 300, function() { 
              $('hr').addClass('icon-up');
            });
          }
        });
      });
    }
  };
}(jQuery));
