(function($) {
  Drupal.behaviors.rawphenoSelTrait = {
    attach: function (context, settings) {
      $(document).ready(function() {
      ///// 
        /*
        var selTrait = $("[name='traits[]']"); 
        $('#edit-download-submit-download').click(function(e) {
          var win = $('#rawdata-window-download');
          
          if(!win.length && selTrait.val() != null) {  
            //add window with timer
            $('#rawpheno-download').prepend('<div id="rawdata-window-download" class="messages status">Download will start <span> </span></div>');
          }
          
          if(win.length) {
            //if timer is present, prevent user from
            //downloading
            alert('File download is in progress');
          }
          else {
            if (selTrait.val() != null) {
              var sec = 3;  
              var timer = setInterval(function() {
                $('#rawdata-window-download span').text(sec);  
                if (sec == 0) {
                  //timer done - clear timer present
                  clearInterval(timer);
                  $('#rawdata-window-download').fadeOut(2000, function() { $(this).remove(); });
                }
                else {
                  sec = sec - 1;
                }
              }, 1000);
            }
          }
        });
        */
        
        //
        var chkb = $('#field-container .frm-cell input:checkbox');
        var selTrait = $("[name='traits[]']");
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