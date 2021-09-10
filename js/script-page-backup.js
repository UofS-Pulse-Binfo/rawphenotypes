/**
 * @file
 * Manage behavior in backup page
 */
 (function($) {
  Drupal.behaviors.rawphenoBackupBehaviours = {
    attach: function (context, settings) {

      
var isAdvancedUpload = function() {
  var div = document.createElement('div');
  return (('draggable' in div) || ('ondragstart' in div && 'ondrop' in div)) && 'FormData' in window && 'FileReader' in window;
}();

var $form = $('.drop-zone');

if (isAdvancedUpload) {

  var droppedFiles = false;
  $form.addClass('has-dragdrop-upload');

  $form.on('drag dragstart dragend dragover dragenter dragleave drop', function(e) {
    e.preventDefault();
    e.stopPropagation();
  })
  .on('dragover dragenter', function() {
    $form.addClass('is-dragover')
  })
  .on('dragleave dragend drop', function() {
     $form.removeClass('is-dragover')
  })
  .on('drop', function(e) {
    droppedFiles = e.originalEvent.dataTransfer.files;
  });
}


var input = document.querySelector('#field-file');
var removeFile = '<a href="#" id="drop-zone-remove-file"><small>[remove]</small></a>' 

input.addEventListener('change', function( e ) {
  var fileName = '';

		if( this.files && this.files.length > 1 ) {
			fileName = ( this.getAttribute( 'data-multiple-caption' ) || '' ).replace( '{count}', this.files.length );
    }
    else {
      fileName = e.target.value.split( '\\' ).pop();
    }

    if( fileName ) {
			document.querySelector('#drop-zone-file').innerHTML = fileName + removeFile;
    }

});





      
      /*
      $('.droppable-browse-button').text('choose your file');

      // Drop area element.
	    var dropZone = document.getElementById('droppable-bdnd');
	    // Initial/default meassage of the drop area.
	    var dropMessage = $('.droppable-message');

	    // Add corresponding message on mouse event.
	    if (dropZone) {
	      // User drags file into the drop area.
		    dropZone.addEventListener('dragover', function() {
		      dropMessage.css('border','3px dashed #AAAAAA');
          // Create a new instruction to user.
          dropMessage.children().hide();
          $('.droppable-message span').eq(0).show().text('Drop to backup your spreadsheet');
	  	  });

		    // User cancels file drop.
		    dropZone.addEventListener('dragleave', function() {
		      dropMessage.css('border','none');
          // Create a new instruction to user.
          $('.droppable-message span').eq(0).text('Drag your Microsoft Excel Spreadsheet file here');
          dropMessage.children().show();
		    });

		    // User drops file.
		    dropZone.addEventListener('drop', function() {
		      dropMessage.css('border','none');
          // Create a new instruction to user.
          $('.droppable-message span').eq(0).text('Drag your Microsoft Excel Spreadsheet file here');
          dropMessage.children().show();
          // AJAX dies or is frozen after an error.
          if ($('div.messages').length) {
            alert();

            // Remove validation result and drupal error message.
            $('div.messages').remove();
          }
		    });
	    }

      // Remove validation result and error messages as soon
      // as DND receives a file. This is for both drag and drop and
      // using the choose a file link (file browser).
      $(document).ajaxStart(function() {
        // AJAX start.
        if ($('div.messages').length) {
          $('div.messages').remove();
        }
      });

      // Reveal validation result text.
      $('div.container-cell').click(function() {
        // Examine the height. When height is 50px, user wants to disclose the entire cell contents
        // else, restore it to initial state.
        var id = $(this).attr('id');
        var i = id.replace(/vn-file-|vr-file-/, '');

        var h = $(this).css('height');
        if (h == '65px') {
          $('#vr-file-' + i).css('height', '100%');

          if ($('#vn-file-' + i)) {
            $('#vn-file-' + i).css('height', '100%');
          }
        }
        else {
          $('#vr-file-' + i).css('height', '65px');

          if ($('#vn-file-' + i)) {
            $('#vn-file-' + i).css('height', '65px');
          }
        }
      });


      // Confirm action.
      $('table a.link-archive, table a.link-delete').click(function(i) {
        var title = i.target.title;

        var r = confirm('Are you sure want to ' + title + '?');
        if (!r) return false;
      });

      // Show/hide archive table.
      $('#container-archive-files a.link-archive').click(function(event) {
        event.preventDefault();
        var archiveTable = $('#tbl_project_archive_file');
        if (archiveTable.is(':hidden')) {
          archiveTable.show();
        }
        else {
          archiveTable.hide();
        }
      })

      // Reload the page when file has no error.
      $(document).ajaxComplete(function() {
        //ajax end
        if ($('.rawpheno-validate-progress').length <= 0 && $('.messages').length <= 0) {
          var link = '../raw/backup/up';
          location.assign(link);
        }
      });


      // For security, suppress any alert message showing snippet of code or
	    // module settings to the user.
	    alert = function(e) {
        // Create an error message.
        $('div.droppable-preview-file').hide();
        $('<div class="messages error">The specified file is not a valid Microsoft Excel File and could not be uploaded.</div>').insertAfter('select');

        return false;
      };
      */
    }
  };
}(jQuery));