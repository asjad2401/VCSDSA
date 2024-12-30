A local version control system implemented in cpp


--------------------------------------------------------------------------------------

.vcs/
    current_branch/
        (current_branch_name).json 
            - name [string]: Name of the current branch.
            - head [string]: Commit ID of the latest commit in the current branch.

    latest_commit/
        (latest_commit_id).json
            - commit_id [string]: The most recent commit ID in the repository.
            - time stamp
    staging/
        files/
            (file_hash)/
                file itself
                (file_name).json
                    - name [string]: Name of the file with extension.
                    - hash [string]: Hash value of the file content.
        tree/
            (stage_id).json
                - directory_tree [JSON object/tree]: JSON representation of the directory structure.

    branches/
        (branch_name).json
            - branch_name [string]: Name of the branch.
            - head [string]: Commit ID of the latest commit in this branch.
            - commits [list of strings]: List of commit IDs in this branch.

    commits/
        commit_timeline.json
            - commits [list of objects]: A timeline of commits with minimal details.
                - commit_id [string]: Commit ID.
                - time [string]: Timestamp of the commit.
                - branch [string]: Branch name.
        (commit_id).json
            - commit_id [string]: Unique identifier for the commit.
            - branch_name [string]: Name of the branch this commit belongs to.
            - directory_structure [JSON object/tree]: Directory tree representation.
            - file_names [list of strings]: List of file names included in the commit.
            - file_hashes [list of strings]: List of corresponding hash values.

    data/
        hash/
            (hash).json
                - file_name [string]: Name of the file with extension.
                - file_hash [string]: Hash value of the file content.
                - branches [list of strings]: Branches that use this file.
                - commit_ids [list of strings]: Commit IDs that reference this file.
            file itself


------------------------------------------------------------------------------------------


init:
- Create an empty `.vcs` folder with placeholders or null objects for:
  - `currentbranch/`
  - `latestcommit/`
  - `staging/`
  - `branches/`
  - `commits/`
  - `data/`

add:
- create stage id if its the very first add (next stage id will be created for the staging after this commit)
- For each file added:
  - Calculate its hash.
  - Store the file and its hash in the staging area.
- Read the current working directory and create/update the directory structure (tree).
- Prepare the staged files for the commit.

first commit:
- Generate a unique commit ID.
- Read file names, hashes, and directory structure from the staging area.
- Create a `commit_id.json`:
  - `commit_id`: The generated commit ID.
  - `branch_name`: "master".
  - `directory_tree`: Structure of the staged files.
  - `file_names`: List of staged file names.
  - `file_hashes`: Corresponding list of file hashes.
- For each file in staging:
  - Create a corresponding data object in `data/`:
    - `file_name`, `hash`, `branch_name master`, and `commit_id`.
- Update the `latest_commit/` file to reflect the new commit ID.
- Update the `branches/`:
  - Create or update `master.json`:
    - `branch_name`: "master".
    - `head`: The commit ID of the new commit.
    - `commits`: A list of commit IDs, adding the new commit ID.
- Update the `current_branch/`:
  - Set `name = "master"`, `head = commit_id`.
- Clear the staging area.

commit (subsequent commits):
- Generate a unique commit ID.
- Read file names, hashes, and directory structure from the staging area.
- Create a `commit_id.json`:
  - `commit_id`: The generated commit ID.
  - `branch_name`: The name of the current branch (from `current_branch/`).
  - `directory_tree`: Structure of the staged files.
  - `file_names`: List of staged file names.
  - `file_hashes`: Corresponding list of file hashes.
- For each file in staging:
  - Create or update a corresponding data object in `data/`:
    - `file_name`, `hash`, `branch_name from current_branch`, and `commit_id`.
- Update the `latest_commit/` to reflect the new commit ID.
- Update the `branches/`:
  - Update the current branch's JSON file:
    - `head`: The new commit ID (update).
    - `commits`: Append the new commit ID to the list of commits.
- Update the `current_branch/`:
  - update `head` to the new commit ID.
- Clear the staging area.

branch:
- Read the latest commit from `latest_commit/` to determine the base for the new branch.
- Update `branches/`:
  - Add a new JSON file for the new branch (e.g., `new_branch_name.json`):
    - `branch_name`: The new branch name.
    - `head`: The latest commit ID (copied from the current branch).
    - `commits`: Copy the commit history from the current branch.
- Update `current_branch/`:
  - Change `name` to the new branch name.

checkout:
- Read the head of the desired branch from `branches/`.
- Use the commit ID from the head to find the corresponding commit in `commits/`.
- Reconstruct the working directory by reading the commit object and its associated data (from `data/`).
- Update `current_branch/` to point to the new branch:
  - Set `name` to the new branch.
  - Set `head` to the latest commit of the new branch.

revert:
- Read the commit object using the provided commit ID.
- Reconstruct the directory tree and committed files from the commit object and `data/`.
- Add all files in the staging area.
- Commit with a message indicating a revert.
  - This will generate a new commit object in `commits/`, linking the current state to the reverted commit.
- Update `branches/` and `current_branch/` to reflect the new commit.

merge:
- Read the names of both the current (target) branch and the given (source) branch.
- The **current branch** (where the user is) is the **target** branch.
- The **given branch** is the **source** branch.
- Get the head of both branches and the last commit for both.
- Find the **common ancestor** commit by comparing the commit histories of both branches.
- Perform a **three-way merge**:
  - Compare changes made in the target branch since the common ancestor.
  - Compare changes made in the source branch since the common ancestor.
  - Merge the changes, resolving conflicts if any (this can be done automatically or with user intervention).
- If there are conflicts, prompt the user to resolve them manually.
- Once conflicts are resolved, stage the changes and commit the merged state:
  - Create a new merge commit in the target branch.
  - Update `branches/`, `current_branch/`, and `commits/` accordingly.
- Update `latest_commit/` to reflect the new merge commit.











