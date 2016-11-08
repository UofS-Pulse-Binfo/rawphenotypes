/**
 * @file
 * Allow user to collapse upload instructions message box and other windows.
 * Notify user to click next step button when user does not response to a stage in
 * upload, for a specific length of time.
 */
(function($) {
  Drupal.behaviors.rawphenoUploadPageElementBehaviors = {
    attach: function (context, settings) {
      // Manage header section of this page.
      var parentContainer = $('div.container-page');
      $(window).resize(function() {
        var w = parentContainer.width();
        var e = $('.subtitle-left');

        var eVal = (w <= 850) ? 1.3 : 1.6;
        e.css('font-size', eVal + 'em');
      });

      // Link to collapse help window.
      var collapseLink = $('#link-help');
      // Container for help text information.
      var helpWindow = $('#container-help-information');
      var speed = 200;

      collapseLink.once(function() {
        $(this).click(function(event) {
          event.preventDefault();
          if (helpWindow.is(':hidden')) {
            helpWindow.slideDown(speed, function() {
              collapseLink.text('Close window');
            });
          }
          else {
            helpWindow.slideUp(speed, function() {
              collapseLink.text('Need help?');
            });
          }
        });
      });

      // Make progress indicator and help window active links.
      $('#container-help-information, div.progress-stage').once(function() {
        $(this).click(function() {
          collapseLink.click();
        });
      });


      // If after 10 seconds and no response from user,
      // button will start to change color and blink.
      var nextButton = $('#edit-next-step');
      // Fieldset container of describe additional trait.
      // If this fieldset is in DOM, don't apply button animation.
      var fieldSet = ($('#edit-xls-review-fldset').length) ? 1 : 0;

      if (nextButton.length > 0 && nextButton.is(':visible')) {
        nextStage();

        if (fieldSet == 0) {
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


      // Count the number of traits checked by user
      // and indicate in the status message box.
      if ($('.form-checkbox').length > 0) {
        var spanTraits = $('#traits-checked');
        var totalNewTraits = spanTraits.text();

        totalNewTraits = $('.form-checkbox:checked').length;
        $('.form-checkbox').once(function() {
          $(this).click(function() {
            if ($(this).is(':checked')) {
              totalNewTraits = parseInt(totalNewTraits) + 1;
            }
            else {
              totalNewTraits = parseInt(totalNewTraits) - 1;
            }
          });
        });

        spanTraits.text(totalNewTraits);
      }

      // Inform user of the next page before clicking the next step
      // button in every stage.
      function nextStage() {
        // Need to figure out what is the current stage and next stage.
        // Fron the progress indicator, count the remaining stages
        var countStagesLeft = $('div.progress-stage-todo').length;
        var nextStage = '';

        if (countStagesLeft == 1) {
          // Completed stages 1 and 2.
          nextStage = 'Stage 3 - Save Spreadsheet';
        }
        else if(countStagesLeft == 2) {
          // Completed stage 1 only.
          nextStage = 'Stage 2 - Describe New Trait';
        }

        $('#container-upload').once(function() {
          $(this).append('<span class="text-next-step">&#x25B8; Next Step: ' + nextStage + '</span>');
        });
      }
    }
  };
}(jQuery));
