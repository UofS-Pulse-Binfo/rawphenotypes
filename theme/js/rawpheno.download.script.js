/**
 * @file 
 * Manage behavior in download form
 */
(function($) {
  Drupal.behaviors.rawphenoSelTrait = {
    attach: function (context, settings) {
      // Reference form elements.
      var chkb = $('input:checkbox');
      var selTrait = $("[name='traits[]']");
   
      selTrait.click(function() {
        // When select all is checked and select trait is clicked.
        // Uncheck the checkbox.
        if (chkb.attr('checked')) { 
         chkb.attr('checked', false);
        }
      });
   
      // Checkbox form element.
      chkb.click(function() {
        // Select all options when checked.
        var state = ($(this).is(':checked')) ? 'selected' : '';
        resetFld(selTrait, state);          
        // When clicked, focus secondary select box.
        $(selTrait).focus();
      });
   
      // Reference form element.
      var btnSubmit = $('#edit-download-submit-download');
   
      // Submit button event with timer.
      btnSubmit.click(function(e) {
        if (selTrait.val() && btnSubmit.val() == 'Download') {
          btnSubmit.val('Download will start in 3');
          var sec = 2;
          var timer = setInterval(function() {
            btnSubmit.val('Download will start in ' + sec);
            if (sec < 0) {
              clearInterval(timer);
              btnSubmit.val('Download');
            }
            else {
              sec = sec - 1;
            }
          }, 1000);
        }
      });
   
      // Disable all form elements when AJAX request in progress,
      // and enable when finished.
      $(document).ajaxStart(function() {
        //ajax start
        $(':input').attr('disabled', 'disabled');
      }).ajaxComplete(function() {
        //ajax end
        resetFld(selTrait, '');
        chkb.attr('checked', false);
        $(':input').removeAttr('disabled');
      });
   
      // Manage error message box.
      var downloadWinError = $('#download-window-error');
      downloadWinError.hide();
   
      if ($('#messages').length > 0) {
        // Check for durpal error message.
        var errorMessage = $('#messages');
        errorMessage.find('h2').remove();
        $('#messages').remove();
        downloadWinError.text(errorMessage.text());
        downloadWinError.show();
      }
      
      // Reset select field when user selects another location.
      function resetFld(select, state) {
        $(select).find('option').each(function() {
          $(this).attr('selected', state);
        });
        //Auto scroll to top of list in select.
        $(select).scrollTop(0);
      }
    }
  };
}(jQuery));