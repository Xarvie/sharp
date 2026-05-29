#include <stdio.h>

/* Test r#"..."# raw string syntax - no escaping needed! */

static const char *html_example = r#"<!DOCTYPE html>
<html>
<head><title>Raw String Test</title></head>
<body>
  <h1>Hello from raw string syntax!</h1>
  <p>This contains "quotes" and \backslashes\ without escaping.</p>
  <script>
    console.log("JavaScript inside raw string!");
  </script>
</body>
</html>"#;

static const char *json_example = r#"{"name": "test", "value": 42, "message": "Hello world"}"#;

int main(void) {
    printf("=== Raw String Test ===\n\n");

    printf("HTML example:\n%s\n\n", html_example);

    printf("JSON example:\n%s\n\n", json_example);

    /* Test r"" syntax (zero hashes) */
    const char *simple = r"simple raw string";
    printf("Simple r\"\" test: %s\n", simple);

    /* Test r##"..."## syntax (multiple hashes) - allows "# inside */
    const char *multi_hash = r##"String with "# and #" inside"##;
    printf("Multi-hash test: %s\n", multi_hash);

    printf("\nAll raw string tests passed!\n");
    return 0;
}
