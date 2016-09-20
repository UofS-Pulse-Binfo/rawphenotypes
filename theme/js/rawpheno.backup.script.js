/**
 * @file
 * Manage behavior in backup page
 */
(function($) {
  Drupal.behaviors.rawphenoBackupBehaviours = {
    attach: function (context, settings) {
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

          if ($('div.file-upload-js-error').length > 0) {
            // AJAX dies or is frozen after an error.
            if ($('div.messages').length > 1) {
              // Remove validation result and drupal error message.
              $('div.messages')[0].remove();
            }
          }
		    });
	    }


      // Remove validation result and error messages as soon
      // as DND receives a file. This is for both drag and drop and
      // using the choose a file link (file browser).
      $(document).ajaxStart(function() {
        // AJAX start.
        if ($('div.messages').length > 1) {
          $('div.messages').remove();
        }
      });

      // Reveal validation result text.
      $('div.container-cell').click(function() {
        // Examine the height. When height is 50px, user wants to disclose the entire cell contents
        // else, restore it to initial state.
        var id = $(this).attr('id');
        var h = $(this).css('height');
        if (h == '65px') {
          $(this).css('height', '100%');
        }
        else {
          $(this).css('height', '65px');
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
    }
  };
}(jQuery));
