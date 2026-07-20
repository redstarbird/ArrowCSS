#include "testing.h"
#include "arrowcss.h"

// Prints a specific depth of indent
static void PrintIndent(int depth)
{
    for (int i = 0; i < depth; i++)
    {
        // 2 spaces for each level of indentation
        printf("  ");
    }
}

static void PrintStringView(struct StringView view)
{
    if (view.length > 0 && view.data != NULL)
    {
        // Print the string view content using its length
        printf("%.*s", (int)view.length, view.data);
    }
}

// Recursive print function
void PrintASTNode(struct ASTNode *node, int depth)
{
    if (node == NULL)
    {
        return;
    }

    struct ASTNode *currentNode = node;
    while (currentNode != NULL)
    {
        PrintIndent(depth);

        switch (currentNode->type)
        {
        case CSS_NODE_STYLESHEET:
            printf("[Stylesheet]\n");

            // Recursively print the stylesheet's children
            PrintASTNode(currentNode->data.stylesheet.children, depth + 1);
            break;
        case CSS_NODE_RULESET:
            printf("[Ruleset] Selector: '");
            PrintStringView(currentNode->data.ruleset.selectors);
            printf("'\n");

            // Recursively print declarations and nested blocks
            PrintASTNode(currentNode->data.ruleset.declarations, depth + 1);
            break;

        case CSS_NODE_AT_RULE:
            // Print at-rule name/identifier
            printf("[At-Rule] ");
            PrintStringView(currentNode->data.at_rule.name);

            // Print at-rule parameters
            if (currentNode->data.at_rule.params.length > 0)
            {
                printf(" (");
                PrintStringView(currentNode->data.at_rule.params);
                printf(")");
            }

            // Recursively print the at-rule's block or ';' if it has no block
            if (currentNode->data.at_rule.block != NULL)
            {
                printf(" {\n");

                PrintASTNode(currentNode->data.at_rule.block, depth + 1);

                PrintIndent(depth);
                printf("}\n");
            }
            else
            {
                printf(" ;\n");
            }

            break;
        case CSS_NODE_DECLARATION:
            printf("[Decl] ");
            PrintStringView(currentNode->data.decl.property);
            printf(": ");
            PrintStringView(currentNode->data.decl.value);

            if (currentNode->data.decl.important)
            {
                printf(" !important");
            }

            printf(";\n");

            break;
        default:
            printf("[Unknown Node]\n");
            break;
        }
        currentNode = currentNode->next;
    }
}

void test_simple_file()
{
    size_t fileSize;
    char *fileData = readDataFromFile("../test/simple.css", &fileSize);

    ASSERT_TRUE(fileData != NULL, "Unable to read file data for simple.css!");

    struct CSSAST *ast = ParseCSSToAST(fileData, fileSize, &(struct ArrowCssParseOptions){.arena = NULL, .pool = NULL, .filename = "simple.css", .generateSourceMap = false, .silent = false});

    free(fileData);
    ArrowCSS_DestroyAST(ast);

    printf("[PASS] test_parse_simple_file\n\n");
}

void test_advanced_file()
{
    size_t fileSize;
    char *fileData = readDataFromFile("../test/advanced.css", &fileSize);

    ASSERT_TRUE(fileData != NULL, "Unable to read file data for simple.css!");

    struct CSSAST *ast = ParseCSSToAST(fileData, fileSize, &(struct ArrowCssParseOptions){.arena = NULL, .pool = NULL, .filename = "simple.css", .generateSourceMap = false, .silent = false});

    free(fileData);
    ArrowCSS_DestroyAST(ast);

    printf("[PASS] test_parse_advanced_file\n\n");
}

int main()
{
    test_simple_file();

    test_advanced_file();

    return 0;
}