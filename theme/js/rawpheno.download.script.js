/**
 * @file
 * Manage behavior in download form
 */
(function($) {
  Drupal.behaviors.rawphenoSelTrait = {
    attach: function (context, settings) {
      // When project is selected.
      // Reset the form - uncheck and unselect.

      // Reference project select box.
      var selPrj = $('#download-sel-project');
      // Reference locations select box, checkbox, and label.
      var locations = {'selectbox': $('#download-sel-location'),
                       'checkbox' : $('#chk-select-all-locations'),
                       'label'    : $('label[for="chk-select-all-locations"]')};

      // Reference traits.
      var traits =    {'selectbox': $('#download-sel-trait'),
                       'checkbox' : $('#chk-select-all-traits'),
                       'label'    : $('label[for="chk-select-all-traits"]')};

      // Add event listener to select a project field.
      selPrj.focus(function() {
        // Reset locations and related fields.
        resetSelect(locations.selectbox, '');
        checkUncheck(locations.checkbox, '');

        // Reset traits and related fields.
        resetSelect(traits.selectbox, '');
        checkUncheck(traits.checkbox, '');
      });

      // Add event listener to locations checkboxes/labels.
      locations.label.add(locations.checkbox).click(function(e) {
        // Check the state of the checkbox and reset or select the select box.
        var s = whatState(locations.checkbox);

        // Support for Safari and IE
        // Source: http://stackoverflow.com/questions/5899783/detect-safari-chrome-ie-firefox-opera-with-user-agent
        //         http://stackoverflow.com/questions/19999388/check-if-user-is-using-ie-with-jquery/21712356#21712356
        var ua = window.navigator.userAgent;
        var old_ie = ua.indexOf('MSIE ');
        var new_ie = ua.indexOf('Trident/');

        if ((navigator.userAgent.indexOf('Safari') != -1 && navigator.userAgent.indexOf('Chrome') == -1) || old_ie > -1 || new_ie > -1) {
          s = !s;
        }

        resetSelect(locations.selectbox, s);
        // Reset the traits select box and uncheck the traits checkbox when this checkbox is clicked.
        resetSelect(traits.selectbox, '');
        checkUncheck(traits.checkbox);

        // Set the focus to locations select box and first option in the list.
        if (s == 'selected') {
          locations.selectbox.scrollTop(0).focus();
        }
      });

      // Add event listener to traits checkboxes/labels
      traits.label.add(traits.checkbox).click(function(e) {
        // Check the state of the checkbox and reset or select the select box.
        var s = whatState(traits.checkbox);

        resetSelect(traits.selectbox, s);
        traits.selectbox.scrollTop(0).focus();
      })

      // Add event listener to select boxes.
      // Locations.
      locations.selectbox.change(function(e) {
        // Reset checkboxes.
        checkUncheck(locations.checkbox);
        checkUncheck(traits.checkbox);
      });

      // Traits.
      traits.selectbox.change(function(e) {
        // Reset checkboxes.
        checkUncheck(traits.checkbox);
      });

      // Environment data file option.
      var env = Drupal.settings.rawpheno.envdata_option;
      $('#chk-envdata').attr('disabled', 'disabled');

      // Disable fields on AJAX (selectbox, checkbox, buttons and all).
      $(document).ajaxStart(function() {
        //ajax start
        $(':input').attr('disabled', 'disabled');
      }).ajaxComplete(function() {
        //ajax end
        resetSelect(traits.selectbox, '')
        $(':input').removeAttr('disabled');

        if (!env) {
          $('#chk-envdata').attr({
            'checked' : false,
            'disabled': 'disabled'
          });
        }
      });


      // Reference form submit button.
      var btnSubmit = $('#edit-download-submit-download');

      // Submit button event with timer.
      // Server side scrip in sync with this timer.
      btnSubmit.click(function(e) {
        $(this).hide();
        $('#div-button').once(function() {
          $(this).append('<span class="win-loading">Please wait...</span>');
        });
      });



      // Function return the state of the checkbox.
      function whatState(fld) {
        return (fld.is(':checked')) ? 'selected' : '';
      }

      // Function reset and select select box.
      function resetSelect(fld, v) {
        $(fld).find('option').each(function() {
          var s = (v == '') ? '' : v;
          $(this).attr('selected', s);
        });
      }

      // Function uncheck and check checkbox.
      function checkUncheck(fld) {
        // Uncheck if it is checked.
        if (fld.is(':checked')) {
          fld.attr('checked', false);
        }
      }

      // Suppress any AJAX error showing snippet of code.
      alert = function(){ };
    }
  };
}(jQuery));
