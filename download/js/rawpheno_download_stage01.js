(function($) {
  Drupal.behaviors.rawphenoSelTrait = {
    attach: function (context, settings) {
      $(document).ready(function() {
      /////  
        $('#edit-download-submit-download').click(function(e) { 
          var win = $('#rawdata-window-download');
          var loc = $('#edit-location').val().toLowerCase();
          
          if(!win.length && $('#edit-sel-'+loc).val() != null) {  
            //add window with timer
            $('#rawpheno-download').prepend('<div id="rawdata-window-download" class="messages status">Download will start <span> </span></div>');
          }
          
          if(win.length) {
            //if timer is present, prevent user from 
            //downloading 
            alert('File download is in progress');
          } 
          else {
            if ($('#edit-sel-'+loc).val() != null) {
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
        
        //ref to location field
        var location = $('#edit-location');
        //ref checkbox
        var chkb = $('#field-container .frm-cell input:checkbox');
  
        chkb.click(function() {
          //select all options
          var fld = '#edit-sel-'+location.val().toLowerCase();
          var state = ($(this).is(':checked')) ? 'selected' : '';
          resetFld(fld, state);          
          //focus on select
          $(fld).focus();
        });
        
        //reset select
        location.change(function() { 
          var fld = '#edit-sel-'+location.val().toLowerCase();
          resetFld(fld, '');
          //uncheck checkbox
          chkb.attr('checked', false);    
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