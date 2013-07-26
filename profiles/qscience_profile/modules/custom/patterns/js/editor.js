window.onload = function() {
	
	(function () {
		
		function addCO2 (textarea, format_selector) {
			if (!textarea) return;
			
			var options = {
		        lineNumbers: true,
		        extraKeys: {
		        	"F11": function(cm) {
		        		setFullScreen(cm, !isFullScreen(cm));
		        	},
		        	"Esc": function(cm) {
		        		if (isFullScreen(cm)) setFullScreen(cm, false);
		        	}
		        },
			    tabMode: "indent",
			    lineWrapping: true
		    };
							
		    var editor = CodeMirror.fromTextArea(textarea, options);
		    var hlLine = editor.addLineClass(0, "background", "activeline");
		    
		    editor.on("cursorActivity", function() {
		    	var cur = editor.getLineHandle(editor.getCursor().line);
		    	if (cur != hlLine) {
		    		editor.removeLineClass(hlLine, "background", "activeline");
		    		hlLine = editor.addLineClass(cur, "background", "activeline");
		    	}
		    }); 
		    
		    //Set the format to be employed by the editor code for this Pattern
			if (typeof format_selector !== 'undefined') {
				editor.setOption('mode', format_selector.value);
			}
		    
		    
		    var fullScreenNotice = 'When cursor is in the editor: F11/ESC toggle full screen editing. Ctrl-F search in the pattern';
		    var noticeNode = document.createTextNode(fullScreenNotice);
		    document.getElementById("edit-content").parentNode.appendChild(noticeNode);
		    
		    
		    function isFullScreen(cm) {
		        return /\bCodeMirror-fullscreen\b/.test(cm.getWrapperElement().className);
		    }
		    
		    function winHeight() {
		        return window.innerHeight || (document.documentElement || document.body).clientHeight;
		    }
		      
		    function setFullScreen(cm, full) {
		        var wrap = cm.getWrapperElement();
		        if (full) {
		        	wrap.className += " CodeMirror-fullscreen";
		        	wrap.style.height = winHeight() + "px";
		        	//document.documentElement.style.overflow = "hidden";
		        } else {
		        	wrap.className = wrap.className.replace(" CodeMirror-fullscreen", "");
		        	wrap.style.height = "";
		        	//document.documentElement.style.overflow = "";
		        }
		        cm.refresh();
		    }
		    
		    CodeMirror.on(window, "resize", function() {
		        var showing = document.body.getElementsByClassName("CodeMirror-fullscreen")[0];
		        if (!showing) return;
		        	showing.CodeMirror.getWrapperElement().style.height = winHeight() + "px";
		    });

		    window.CO2editor = editor;
		}
		
		
		addCO2(document.getElementById('edit-content-db'), document.getElementById('edit-format-db'));
		addCO2(document.getElementById('edit-content'), document.getElementById('edit-format'));
		
	})();
	
};