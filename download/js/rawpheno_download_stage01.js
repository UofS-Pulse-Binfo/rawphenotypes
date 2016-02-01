(function($) {
  Drupal.behaviors.rawphenoSelTrait = {
    attach: function (context, settings) {
      $(document).ready(function() {
      ///// 
        var chkb = $('#field-container .frm-cell input:checkbox');
        var selTrait = $("[name='traits[]']");
        var btnSubmit = $('#edit-download-submit-download');
        
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

        chkb.click(function() {
          //select all options
          var state = ($(this).is(':checked')) ? 'selected' : '';
          resetFld(selTrait, state);          
          //focus on select
          $(selTrait).focus();
        });
        
        //Disable all form elements when ajax request in progress.
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
      
      //select/deselect select fld
      function resetFld(select, state) {
        $(select).find('option').each(function() {
          $(this).attr('selected', state);
        });
        //scroll back to top
        $(select).scrollTop(0);
      }
    }
  };
}(jQuery));