$(document).ready(function() {
	$(".ToggleButton").click(function(event) {
		var jq = $(this);
		jq.next("div:first").toggle();

	});
});