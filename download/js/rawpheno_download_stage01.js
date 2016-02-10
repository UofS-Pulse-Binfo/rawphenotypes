/**
 * @file 
 * Manage behavior in download form
 */
(function($) {
  Drupal.behaviors.rawphenoSelTrait = {
    attach: function (context, settings) {
      $(document).ready(function() {
      /////
        // Reference form elements.
        var chkb = $('#field-container .frm-cell input:checkbox');
        var selTrait = $("[name='traits[]']");
        var btnSubmit = $('#edit-download-submit-download');
        
        // Submit button event
        btnSubmit.click(function(e) { 
          if (selTrait.val() && btnSubmit.val() == 'Download') {
            btnSubmit.val('Download in 3');
            var sec = 2;
            var timer = setInterval(function() {
              btnSubmit.val('Download in ' + sec);
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

        // Checkbox form element
        chkb.click(function() {
          // Select all options when checked.
          var state = ($(this).is(':checked')) ? 'selected' : '';
          resetFld(selTrait, state);          
          // When clicked, focus secondary select box.
          $(selTrait).focus();
        });
        
        // Disable all form elements when ajax request in progress.
        $(document).ajaxStart(function() {
          //ajax start
          chkb.attr('checked', false); 
          $(':input').attr('disabled', 'disabled');
        }).ajaxComplete(function() {
          //ajax end
          resetFld(selTrait, '');
          $(':input').removeAttr('disabled');
        });
      /////    
      });
      
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