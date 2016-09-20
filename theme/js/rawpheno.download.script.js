/**
 * @file
 * Manage behavior in download form
 */
(function($) {
  Drupal.behaviors.rawphenoSelTrait = {
    attach: function (context, settings) {
      // Reference all select elements.
      var sel = $('select');

      // Use these vars to reference location and trait select boxes
      // instead of index 1 and index 2, respectively.
      var locations = 1, traits = 2;

      // Reference all checkboxes.
      var chkb = $('input:checkbox:lt(2)');

      // When user click the select project select box, reset the form
      // by removing any default/user selected option and uncheck the checkboxes.
      $('#download-sel-project').click(function() {
        // Reset select boxes.
        resetSelect(sel[locations], '');
        resetSelect(sel[traits], '');

        // Reset checkboxes.
        chkb.attr('checked', false);
      });

      // Add event listener to checkboxes.
      chkb.click(function(i) {
        // id attribute of the checked field.
        var id = (i.target.id.indexOf('location') > 0) ? locations : traits;
        // check/uncheck checkbox.
        var state = ($(this).is(':checked')) ? 'selected' : '';
        resetSelect(sel[id], state);
        $(sel[id]).scrollTop(0).focus().click();
      });

      // Add event listener to select trait select boxe.
      // Uncheck the select all option when select box is clicked/changed.
      $(sel[traits]).change(function() {
        chkb.eq(1).attr('checked', false);
      });


      // Reference form submit button.
      var btnSubmit = $('#edit-download-submit-download');

      // Submit button event with timer.
      btnSubmit.click(function(e) {
        if (btnSubmit.val() == 'Download') {
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

      // Disable fields on AJAX (selectbox, checkbox, buttons and all).
      $(document).ajaxStart(function() {
        //ajax start
        $(':input').attr('disabled', 'disabled');
      }).ajaxComplete(function() {
        //ajax end
        resetSelect(sel[traits], '')
        chkb.attr('checked', false);
        $(':input').removeAttr('disabled');
      });


      // Function reset select box.
      function resetSelect(select, state) {
        $(select).find('option').each(function() {
          var s = (state == '') ? '' : state;
          $(this).attr('selected', s);
        });
      }
    }
  };
}(jQuery));
