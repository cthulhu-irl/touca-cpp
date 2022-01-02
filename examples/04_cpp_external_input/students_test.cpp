// Copyright 2021 Touca, Inc. Subject to Apache-2.0 License.

#include "students_test.hpp"

#include <iostream>
#include <thread>

#include "cxxopts.hpp"
#include "students.hpp"
#include "touca/core/filesystem.hpp"
#include "touca/runner/runner.hpp"
#include "touca/touca.hpp"

int main(int argc, char* argv[]) {
  MyWorkflow workflow;
  return touca::main(argc, argv, workflow);
}

MySuite::MySuite(const touca::FrameworkOptions& options) : _options(options) {}

void MySuite::initialize() {
  // if option `testsuite-file` is specified, use the testcases listed
  // in that file.
  if (_options.extra.count("testsuite-file")) {
    for (const auto& testcase :
         touca::get_testsuite_local(_options.extra.at("testsuite-file"))) {
      push(testcase);
    }
  }
  // if option `testsuite-remote` is specified, use the testcases that are
  // part of the version submitted to the Touca server that is currently
  // the suite baseline.
  if (_options.extra.count("testsuite-remote") &&
      _options.extra.at("testsuite-remote") == "true") {
    for (const auto& testcase : touca::get_testsuite_remote(_options)) {
      push(testcase);
    }
  }
  // if neither options are provided, use all the profiles that exist in
  // the datasets directory as testcases.
  for (const auto& it : touca::filesystem::directory_iterator(
           _options.extra.at("datasets-dir"))) {
    if (!touca::filesystem::is_regular_file(it.path().string())) {
      continue;
    }
    push(it.path().stem().string());
  }
}

cxxopts::Options application_options() {
  cxxopts::Options options{""};
  // clang-format off
    options.add_options()
        ("datasets-dir", "path to datasets directory", cxxopts::value<std::string>())
        ("testsuite-file", "path to testsuite file", cxxopts::value<std::string>())
        ("testsuite-remote", "reuse testcases of baseline\n", cxxopts::value<std::string>()->implicit_value("true"));
  // clang-format on
  return options;
}

MyWorkflow::MyWorkflow() : touca::Workflow() {}

std::string MyWorkflow::describe_options() const {
  return application_options().help();
}

bool MyWorkflow::parse_options(int argc, char* argv[]) {
  auto options = application_options();
  options.allow_unrecognised_options();
  const auto& result = options.parse(argc, argv);
  for (const auto& key :
       {"datasets-dir", "testsuite-file", "testsuite-remote"}) {
    if (result.count(key)) {
      _options.extra[key] = result[key].as<std::string>();
    }
  }
  return true;
}

bool MyWorkflow::validate_options() const {
  // check that option `datasets-dir` is provided.

  if (!_options.extra.count("datasets-dir")) {
    std::cerr << "required configuration option \"datasets-dir\" is missing"
              << std::endl;
    return false;
  }

  // check that directory pointed by option `datasets-dir` exists.

  const auto& datasetsDir = _options.extra.at("datasets-dir");
  if (!touca::filesystem::is_directory(datasetsDir)) {
    std::cerr << "datasets directory \"" << datasetsDir << "\" does not exist"
              << std::endl;
    return false;
  }

  // if option `testsuite-file` is provided, check that it points to a valid
  // file.

  if (_options.extra.count("testsuite-file")) {
    const auto& file = _options.extra.at("testsuite-file");
    if (!touca::filesystem::is_regular_file(file)) {
      std::cerr << "testsuite file \"" << file << "\" does not exist"
                << std::endl;
      return false;
    }
  }

  return true;
}

std::shared_ptr<touca::Suite> MyWorkflow::suite() const {
  return std::make_shared<MySuite>(_options);
}

std::vector<std::string> MyWorkflow::execute(
    const std::string& testcase) const {
  touca::filesystem::path caseFile = _options.extra.at("datasets-dir");
  caseFile /= testcase + ".json";
  const auto& student = find_student(caseFile.string());

  touca::assume("username", student.username);
  touca::check("fullname", student.fullname);
  touca::check("birth_date", student.dob);
  touca::check("gpa", calculate_gpa(student.courses));

  custom_function_1(student);

  std::thread t(custom_function_2, student);
  t.join();

  touca::start_timer("func3");
  custom_function_3(student);
  touca::stop_timer("func3");

  touca::add_metric("external", 10);
  return {};
}

template <>
struct touca::serializer<Date> {
  data_point serialize(const Date& value) {
    return object("Date")
        .add("year", value.year)
        .add("month", value.month)
        .add("day", value.day);
  }
};
