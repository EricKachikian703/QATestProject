#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <sstream>
#include <exception>
#include <cctype>

// ======================================================
// Simple QA Testing Framework Starter Project
// ======================================================

// ------------------------------
// Test result tracking
// ------------------------------
struct TestStats {
    int testsRun = 0;
    int testsPassed = 0;
    int testsFailed = 0;
    int assertionsRun = 0;
    int assertionsFailed = 0;
};

TestStats g_stats;

// ------------------------------
// Custom exception for ASSERT_*
// ------------------------------
class AssertFailure : public std::exception {
private:
    std::string message;

public:
    explicit AssertFailure(const std::string& msg) : message(msg) {}

    const char* what() const noexcept override {
        return message.c_str();
    }
};

// ------------------------------
// Test case structure
// ------------------------------
struct TestCase {
    std::string name;
    std::function<void()> func;
};

// Global test registry
std::vector<TestCase>& getTestRegistry() {
    static std::vector<TestCase> tests;
    return tests;
}

// Register test helper
void registerTest(const std::string& name, std::function<void()> func) {
    getTestRegistry().push_back({ name, func });
}

// ------------------------------
// Macro to define tests
// ------------------------------
#define TEST(test_name)                                      \
    void test_name();                                        \
    struct test_name##_registrar {                           \
        test_name##_registrar() {                            \
            registerTest(#test_name, test_name);             \
        }                                                    \
    };                                                       \
    static test_name##_registrar test_name##_instance;       \
    void test_name()

// ------------------------------
// Assertion helpers
// ------------------------------
void recordAssertion() {
    g_stats.assertionsRun++;
}

void recordFailedAssertion() {
    g_stats.assertionsFailed++;
}

template <typename T, typename U>
std::string makeComparisonMessage(
    const std::string& exprA,
    const std::string& exprB,
    const T& actual,
    const U& expected,
    const std::string& file,
    int line,
    const std::string& op
) {
    std::ostringstream oss;
    oss << file << ":" << line << "\n"
        << "  Assertion failed: " << exprA << " " << op << " " << exprB << "\n"
        << "  Actual:   " << actual << "\n"
        << "  Expected: " << expected;
    return oss.str();
}

std::string makeBoolMessage(
    const std::string& expr,
    const std::string& file,
    int line
) {
    std::ostringstream oss;
    oss << file << ":" << line << "\n"
        << "  Assertion failed: " << expr;
    return oss.str();
}

// ------------------------------
// EXPECT macros (continue test)
// ------------------------------
#define EXPECT_TRUE(condition)                                                   \
    do {                                                                         \
        recordAssertion();                                                       \
        if (!(condition)) {                                                      \
            recordFailedAssertion();                                             \
            std::cerr << __FILE__ << ":" << __LINE__ << "\n"                     \
                      << "  EXPECT_TRUE failed: " << #condition << "\n";         \
        }                                                                        \
    } while (0)

#define EXPECT_FALSE(condition)                                                  \
    do {                                                                         \
        recordAssertion();                                                       \
        if ((condition)) {                                                       \
            recordFailedAssertion();                                             \
            std::cerr << __FILE__ << ":" << __LINE__ << "\n"                     \
                      << "  EXPECT_FALSE failed: " << #condition << "\n";        \
        }                                                                        \
    } while (0)

#define EXPECT_EQ(actual, expected)                                              \
    do {                                                                         \
        recordAssertion();                                                       \
        auto _actual = (actual);                                                 \
        auto _expected = (expected);                                             \
        if (!(_actual == _expected)) {                                           \
            recordFailedAssertion();                                             \
            std::cerr << makeComparisonMessage(#actual, #expected,               \
                        _actual, _expected, __FILE__, __LINE__, "==")            \
                      << "\n";                                                   \
        }                                                                        \
    } while (0)

#define EXPECT_NE(actual, expected)                                              \
    do {                                                                         \
        recordAssertion();                                                       \
        auto _actual = (actual);                                                 \
        auto _expected = (expected);                                             \
        if (!(_actual != _expected)) {                                           \
            recordFailedAssertion();                                             \
            std::cerr << makeComparisonMessage(#actual, #expected,               \
                        _actual, _expected, __FILE__, __LINE__, "!=")            \
                      << "\n";                                                   \
        }                                                                        \
    } while (0)

// ------------------------------
// ASSERT macros (stop test)
// ------------------------------
#define ASSERT_TRUE(condition)                                                   \
    do {                                                                         \
        recordAssertion();                                                       \
        if (!(condition)) {                                                      \
            recordFailedAssertion();                                             \
            throw AssertFailure(makeBoolMessage(#condition, __FILE__, __LINE__));\
        }                                                                        \
    } while (0)

#define ASSERT_EQ(actual, expected)                                              \
    do {                                                                         \
        recordAssertion();                                                       \
        auto _actual = (actual);                                                 \
        auto _expected = (expected);                                             \
        if (!(_actual == _expected)) {                                           \
            recordFailedAssertion();                                             \
            throw AssertFailure(makeComparisonMessage(#actual, #expected,        \
                _actual, _expected, __FILE__, __LINE__, "=="));                  \
        }                                                                        \
    } while (0)

// ======================================================
// Sample module under test: Login Validator
// ======================================================
namespace LoginValidator {

    bool isValidUsername(const std::string& username) {
        if (username.length() < 3 || username.length() > 15) {
            return false;
        }

        for (char c : username) {
            if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') {
                return false;
            }
        }

        return true;
    }

    bool isValidPassword(const std::string& password) {
        if (password.length() < 8) {
            return false;
        }

        bool hasUpper = false;
        bool hasLower = false;
        bool hasDigit = false;
        bool hasSpecial = false;

        for (char c : password) {
            if (std::isupper(static_cast<unsigned char>(c))) {
                hasUpper = true;
            }
            else if (std::islower(static_cast<unsigned char>(c))) {
                hasLower = true;
            }
            else if (std::isdigit(static_cast<unsigned char>(c))) {
                hasDigit = true;
            }
            else {
                hasSpecial = true;
            }
        }

        return hasUpper && hasLower && hasDigit && hasSpecial;
    }

    class LoginSystem {
    private:
        std::string storedUsername;
        std::string storedPassword;
        int failedAttempts;
        bool locked;

    public:
        LoginSystem(const std::string& user, const std::string& pass)
            : storedUsername(user), storedPassword(pass), failedAttempts(0), locked(false) {}

        bool authenticate(const std::string& user, const std::string& pass) {
            if (locked) {
                return false;
            }

            if (user == storedUsername && pass == storedPassword) {
                failedAttempts = 0;
                return true;
            }

            failedAttempts++;
            if (failedAttempts >= 3) {
                locked = true;
            }

            return false;
        }

        bool isLockedOut() const {
            return locked;
        }

        int getFailedAttempts() const {
            return failedAttempts;
        }

        void resetLockout() {
            failedAttempts = 0;
            locked = false;
        }
    };

} // namespace LoginValidator

// ======================================================
// Tests
// ======================================================

TEST(ValidUsernameAccepted) {
    EXPECT_TRUE(LoginValidator::isValidUsername("Eric_123"));
}

TEST(UsernameTooShortRejected) {
    EXPECT_FALSE(LoginValidator::isValidUsername("ab"));
}

TEST(UsernameWithInvalidSymbolRejected) {
    EXPECT_FALSE(LoginValidator::isValidUsername("Eric!"));
}

TEST(ValidPasswordAccepted) {
    EXPECT_TRUE(LoginValidator::isValidPassword("Strong1!"));
}

TEST(PasswordMissingUppercaseRejected) {
    EXPECT_FALSE(LoginValidator::isValidPassword("weakpass1!"));
}

TEST(PasswordMissingDigitRejected) {
    EXPECT_FALSE(LoginValidator::isValidPassword("Weakpass!"));
}

TEST(PasswordTooShortRejected) {
    EXPECT_FALSE(LoginValidator::isValidPassword("Aa1!"));
}

TEST(LoginSucceedsWithCorrectCredentials) {
    LoginValidator::LoginSystem system("admin", "Password1!");

    EXPECT_TRUE(system.authenticate("admin", "Password1!"));
    EXPECT_EQ(system.getFailedAttempts(), 0);
    EXPECT_FALSE(system.isLockedOut());
}

TEST(LoginFailsWithWrongPassword) {
    LoginValidator::LoginSystem system("admin", "Password1!");

    EXPECT_FALSE(system.authenticate("admin", "WrongPass1!"));
    EXPECT_EQ(system.getFailedAttempts(), 1);
    EXPECT_FALSE(system.isLockedOut());
}

TEST(AccountLocksAfterThreeFailures) {
    LoginValidator::LoginSystem system("admin", "Password1!");

    EXPECT_FALSE(system.authenticate("admin", "bad1"));
    EXPECT_FALSE(system.authenticate("admin", "bad2"));
    EXPECT_FALSE(system.authenticate("admin", "bad3"));

    EXPECT_EQ(system.getFailedAttempts(), 3);
    EXPECT_TRUE(system.isLockedOut());
}

TEST(ResetLockoutWorks) {
    LoginValidator::LoginSystem system("admin", "Password1!");

    system.authenticate("admin", "bad1");
    system.authenticate("admin", "bad2");
    system.authenticate("admin", "bad3");

    ASSERT_TRUE(system.isLockedOut());

    system.resetLockout();

    EXPECT_FALSE(system.isLockedOut());
    EXPECT_EQ(system.getFailedAttempts(), 0);
    EXPECT_TRUE(system.authenticate("admin", "Password1!"));
}

// ======================================================
// Test runner
// ======================================================
int runAllTests() {
    auto& tests = getTestRegistry();

    std::cout << "========================================\n";
    std::cout << " Running " << tests.size() << " tests\n";
    std::cout << "========================================\n\n";

    for (const auto& test : tests) {
        g_stats.testsRun++;

        int failedBefore = g_stats.assertionsFailed;

        try {
            test.func();

            int failedAfter = g_stats.assertionsFailed;
            if (failedAfter == failedBefore) {
                g_stats.testsPassed++;
                std::cout << "[PASS] " << test.name << "\n";
            }
            else {
                g_stats.testsFailed++;
                std::cout << "[FAIL] " << test.name << "\n";
            }
        }
        catch (const AssertFailure& ex) {
            g_stats.testsFailed++;
            std::cout << "[FAIL] " << test.name << "\n";
            std::cerr << ex.what() << "\n";
        }
        catch (const std::exception& ex) {
            g_stats.testsFailed++;
            std::cout << "[ERROR] " << test.name << "\n";
            std::cerr << "  Unexpected exception: " << ex.what() << "\n";
        }
        catch (...) {
            g_stats.testsFailed++;
            std::cout << "[ERROR] " << test.name << "\n";
            std::cerr << "  Unknown exception occurred.\n";
        }
    }

    std::cout << "\n========================================\n";
    std::cout << " Test Summary\n";
    std::cout << "========================================\n";
    std::cout << "Tests run:         " << g_stats.testsRun << "\n";
    std::cout << "Tests passed:      " << g_stats.testsPassed << "\n";
    std::cout << "Tests failed:      " << g_stats.testsFailed << "\n";
    std::cout << "Assertions run:    " << g_stats.assertionsRun << "\n";
    std::cout << "Assertions failed: " << g_stats.assertionsFailed << "\n";
    std::cout << "========================================\n";

    return (g_stats.testsFailed == 0) ? 0 : 1;
}

// ======================================================
// Main
// ======================================================
int main() {
    return runAllTests();
}