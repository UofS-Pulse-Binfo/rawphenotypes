/**
 * @file 
 * Manage behavior in download form
 */
(function($) {
  Drupal.behaviors.rawphenoSelTrait = {
    attach: function (context, settings) {
      // Reference form elements.
      // checkboxes to select location and traits.
      var chkb = $('input:checkbox');
      // Select fields for location and traits.
      var sel = $('select');
      // Location and traits fields.
      var locations = 0, traits = 1;
      
      sel.change(function() {
        var i;
        // When either select field is clicked, uncheck its checkbox.
        i = ($(this).attr('name') == 'location[]') ? locations : traits;
        chkb.eq(i).attr('checked', false);
      });
    
      // Select/unselect all options in a select on check/uncheck of its checkbox. 
      chkb.click(function() { 
        // Determine if checkbox is checked or unchecked
        // and select or unselect accordingly.
        var state = ($(this).is(':checked')) ? 'selected' : '';

        var i;
        // Which checkbox is clicked.
        i = ($(this).attr('name').indexOf('location') > 0) ? locations : traits;
        resetFld(sel[i], state);
        
        // When select all location is clicked, call AJAX
        // that updates list of traits.
        if (i == locations && state == 'selected') {
          $(sel[locations]).change();
          chkb.eq(locations).attr('checked', 'checked');
        }
        else if(i == locations && state == '') {
          // Default to first option in the select and refresh traits field
          // when select all locations is unchecked.
          $(sel[locations] + 'option:first-child').attr('selected', 'selected');
          $(sel[locations]).change();
        }
      });
      
      // Reference form element.
      var btnSubmit = $('#edit-download-submit-download');
   
      // Submit button event with timer.
      btnSubmit.click(function(e) {
        if ($(sel[traits]).val() && btnSubmit.val() == 'Download') {
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
      
      // Disable all form elements when AJAX request in progress,
      // and enable and reset fields when finished.
      $(document).ajaxStart(function() {
        //ajax start
        $(':input').attr('disabled', 'disabled');
      }).ajaxComplete(function() {
        //ajax end
        resetFld(sel[traits], '');
        chkb.eq(traits).attr('checked', false);
        $(':input').removeAttr('disabled');
      });
      
      
      // Function to select and unselect options of a given select field.
      function resetFld(select, state) {
        $(select).find('option').each(function() {
          $(this).attr('selected', state);
        });
        //Auto scroll to top of list and focus field.
        $(select).scrollTop(0).focus();
      }
    }
  };
}(jQuery));