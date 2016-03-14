/**
 * @file
 * Allow user to collapse upload instructions message box and other windows.
 */
(function($) {
  Drupal.behaviors.rawphenoToggleWindows = {
    attach: function (contex, settings) {
      // Allow user to collapse upload instructions message box.
      $('hr').click(function() {
        var helpTextWindow = $('#txt-help-text');
     
        if ($(this).attr('class') == 'icon-up') {
          // Help window text open.
          helpTextWindow.stop().animate({height : '140px'}, 300, function() { 
            $('hr').removeClass('icon-up');
          });
        } 
        else {
          // Help window text collapse.
          helpTextWindow.stop().animate({height : '0px'}, 300, function() { 
            $('hr').addClass('icon-up');
          });
        }
      });
         
      // Allow user to close error window
      $('h3 a').click(function(event) {
        event.preventDefault();
        var validationResultWindow = $('div.rawpheno-validate-progress');
        
        if ($(this).text() == 'Close window') {
          validationResultWindow.stop().animate({height: '44px'}, 300, function() {
            $('h3 a').text('Open window');
          });
        }
        else {
          validationResultWindow.stop().animate({height: '100%'}, 300, function() {
            $('h3 a').text('Close window');
          });
        }
      });
    }
  };
}(jQuery));