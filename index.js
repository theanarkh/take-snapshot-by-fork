const addon = require('./build/Release/take_snapshot_by_fork.node');

module.exports = {
    takeSnapshotByFork: addon.takeSnapshotByFork
};