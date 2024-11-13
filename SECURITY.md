# Security Policy

If you discover a security-related issue (a crash), please report it based on the instructions below.

## Reporting a Vulnerability

Please **DO NOT** file a public issue, instead report the vulnerability on the relevant
[GitHub security](https://github.com/CESNET/libyang/security) page. If you do not receive any reaction within 48 hours,
please also send an email to [mvasko@cesnet.cz].

## Review Process

After receiving the report, an initial triage and technical analysis is performed to confirm the report and determine
its scope. We may request additional information in this stage of the process.

Once a reviewer has confirmed the relevance of the report, a draft security advisory will be created on GitHub. The
draft advisory will be used to discuss the issue with maintainers, the reporter(s), and where applicable, other affected
parties under embargo.

If the vulnerability is accepted, a timeline for developing a patch, public disclosure, and patch release will be
determined. If there is an embargo period on public disclosure before the patch release, the reporter(s) are expected to
participate in the discussion of the timeline and abide by agreed upon dates for public disclosure.

Usually, the reasonably complex issues are fixed within hours of being reported.

## Supported Versions

After an issue is fixed, it **WILL NOT** be backported to any released version. Instead, it is kept in the public `devel`
branch, which is periodically merged into the main branch when a new release is due. So, the issue will be fixed in the
next release after it is fixed.
