# Pull Request Process

This guide explains how to submit changes to Protocol Buffers.

## Before You Start

1. **Check for existing work** - Search issues and PRs
2. **Discuss major changes** - Open an issue first
3. **Sign the CLA** - Required for all contributions

## Creating a Pull Request

### 1. Fork and Clone

```bash
# Fork on GitHub, then clone
git clone https://github.com/YOUR_USERNAME/protobuf.git
cd protobuf

# Add upstream remote
git remote add upstream https://github.com/protocolbuffers/protobuf.git
```

### 2. Create a Branch

```bash
# Update main
git fetch upstream
git checkout main
git merge upstream/main

# Create feature branch
git checkout -b feature/my-change
```

### 3. Make Changes

- Follow [code style](style.md)
- Write tests
- Update documentation

### 4. Commit

```bash
git add .
git commit -m "component: Brief description

Detailed explanation of the change.

Fixes #123"
```

### 5. Push

```bash
git push origin feature/my-change
```

### 6. Open PR

1. Go to your fork on GitHub
2. Click "New Pull Request"
3. Select your branch
4. Fill out the template

## PR Template

```markdown
## Description

Brief description of the change.

## Motivation

Why is this change needed?

## Changes

- List of changes made
- Another change

## Testing

How was this tested?

- [ ] Unit tests added
- [ ] Existing tests pass
- [ ] Manually tested

## Related Issues

Fixes #123
```

## Code Review

### What Reviewers Check

- **Correctness** - Does it work?
- **Style** - Does it follow guidelines?
- **Tests** - Is it well tested?
- **Docs** - Is it documented?
- **Performance** - Any regressions?

### Responding to Feedback

1. **Be responsive** - Reply promptly
2. **Be open** - Consider all feedback
3. **Ask questions** - Clarify if unclear
4. **Update code** - Push new commits

```bash
# Make requested changes
git add .
git commit -m "Address review feedback"
git push origin feature/my-change
```

### Resolving Conflicts

```bash
# Update your branch
git fetch upstream
git rebase upstream/main

# Resolve conflicts
git add .
git rebase --continue

# Force push
git push -f origin feature/my-change
```

## CI Checks

All PRs must pass CI checks:

- **Build** - Code compiles
- **Tests** - All tests pass
- **Style** - Code is formatted
- **Coverage** - No major regression

### Fixing CI Failures

1. Check the CI logs
2. Reproduce locally
3. Fix and push

## Approval and Merge

### Requirements

- [ ] CLA signed
- [ ] CI passing
- [ ] Approved by reviewer
- [ ] No unresolved comments

### Merge Process

A maintainer will merge your PR when ready. We use squash merge to keep history clean.

## After Merge

### Cleanup

```bash
# Delete local branch
git checkout main
git branch -d feature/my-change

# Delete remote branch
git push origin --delete feature/my-change
```

### Update Your Fork

```bash
git fetch upstream
git checkout main
git merge upstream/main
git push origin main
```

## Common Scenarios

### Small Bug Fix

1. Create branch from main
2. Fix bug
3. Add test
4. Open PR

### New Feature

1. Open issue to discuss
2. Get approval
3. Implement with tests
4. Open PR referencing issue

### Documentation Update

1. Create branch
2. Make changes
3. Preview locally (mkdocs serve)
4. Open PR

## Tips for Success

### Do

- Keep PRs small and focused
- Write clear descriptions
- Respond to feedback quickly
- Be patient with review process

### Don't

- Mix unrelated changes
- Skip tests
- Ignore review comments
- Force push without warning

## Getting Help

- Comment on your PR
- Ask in [Discussions](https://github.com/protocolbuffers/protobuf/discussions)
- Check existing PRs for examples

## See Also

- [Development Setup](setup.md)
- [Code Style](style.md)
- [Testing Guide](testing.md)
